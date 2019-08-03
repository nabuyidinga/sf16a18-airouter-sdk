/**
 *****************************************************************************************
 * @file rw_comm.c
 *
 * @brief Main file for rw application, sending DBG commands to LMAC layer
 *
 * Copyright (C) RivieraWaves 2011-2012
 *
 *****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * DEFINES
 *****************************************************************************************
 */
//the unit is ps, our LA always run in 150MHz
#define DELAY (unsigned long long)6667

struct group {
    char name[255];
    char id[15];
    int lsb;
    int msb;
    uint32_t prev;
};

struct groups {
    int cnt;
    struct group group[32]; // Up to 32 groups per 32-bit bank
};

/*
 * FUNCTION DEFINITIONS
 *****************************************************************************************
 */
static int int2bin(char *out, uint32_t val, int width, uint32_t smp_msk)
{
    int status = -1;

    out[0] = '\0';

    uint32_t z = 1 << (width - 1);
    for (; z > 0; z >>= 1)
    {
        if (z & smp_msk)
        {
            strcat(out, ((val & z) == z) ? "1" : "0");
            status = 0;
        }
        else
            strcat(out, "x");
    }

    return status;
}

static ssize_t readline(char **lineptr, FILE *fp)
{
    *lineptr = NULL;
    char *line;
    size_t len = 0;
    ssize_t read;
    int i;

    read = getline(lineptr, &len, fp);

    if (read <= 0)
        return read;

    line = *lineptr;

    // Remove the end of line character
    if (line[read - 1] == '\n')
        line[read - 1] = '\0';

    // Keep only the characters up to a * or a [
    for (i = 0; i < strlen(line); i++)
    {
        if ((line[i] == '*') || (line[i] == '['))
        {
            line[i] = '\0';
            break;
        }
    }

    return strlen(line);
}

static int add_group(struct groups *groups, char *name, char *id_prefix, int lsb, int msb, uint32_t val)
{
    struct group *g;

    if (groups->cnt == 32)
        return -1;

    if (msb >= 32)
        return -1;

    g = &groups->group[groups->cnt];

    strcpy(g->name, name);
    sprintf(g->id, "%s%d", id_prefix, groups->cnt);
    g->lsb = lsb;
    g->msb = msb;
    g->prev = val;

    groups->cnt++;

    return 0;
}

static int enlarge_group(struct groups *groups)
{
    struct group *g;

    if (groups->cnt == 0)
        return -1;

    g = &groups->group[groups->cnt - 1];

    if (g->msb >= 32)
        return -1;

    g->msb++;

    return 0;
}

static FILE *hwdiag_name_open(char *path, char *name)
{
    FILE *fp;
    char filename[256];

    snprintf(filename, 255, "%s/%s", path, name);
    fp = fopen(filename, "r");

    return fp;
}

static FILE *hwdiag_config_open(char *path)
{
    FILE *fp;
    char filename[256];

    snprintf(filename, 255, "%s/hwdiags", path);
    fp = fopen(filename, "r");

    return fp;
}

static FILE *plfdiag_config_open(char *path)
{
    FILE *fp;
    char filename[256];

    snprintf(filename, 255, "%s/plfdiags", path);
    fp = fopen(filename, "r");

    return fp;
}

static FILE *vcd_open(char *path)
{
    FILE *fp;
    char filename[256];

    snprintf(filename, 255, "%s/trace.vcd", path);
    fp = fopen(filename, "w");

    return fp;
}

static FILE *trace_open(char *path)
{
    FILE *fp;
    char filename[256];

    snprintf(filename, 255, "%s/mactrace", path);
    fp = fopen(filename, "r");

    return fp;
}

static int get_la_conf(uint32_t *sampling, uint32_t *version, char *path, char *name)
{
    FILE *fp;
    char filename[256];
    int i;

    snprintf(filename, 255, "%s/%s", path, name);
    fp = fopen(filename, "r");
    if (fp == NULL)
        return -1;

    // Get sampling mask
    for (i=0; i<3; i++)
    {
        if (fread(&sampling[i], 4, 1, fp) != 1)
            break;
    }

    // Get unused words
    for (i=0; i<6; i++)
    {
        uint32_t dummy;
        if (fread(&dummy, 4, 1, fp) != 1)
            break;
    }

    // Get version
    if (fread(version, 4, 1, fp) != 1)
        *version = 0;

    fclose (fp);
    return 0;
}

static int create_swdiag_groups(struct groups *swdiags, char *path)
{
    FILE *fp;
    char filename[256];
    int i = 0;
    char line_prev[256] = "";

    printf("Create SW diag groups\n");

    swdiags->cnt = 0;

    snprintf(filename, 255, "%s/swdiags", path);
    fp = fopen(filename, "r");

    if (fp == NULL)
        return -1;

    while (1)
    {
        char *line;
        ssize_t read;
        read = readline(&line, fp);

        if (read <= 0)
            break;

        // Compare with previous diag name to know if it is part of a group
        if (strcmp(line_prev, line))
        {
            /* Create new group */
            add_group(swdiags, line, "sw", i, i, 0);
            strcpy(line_prev, line);
        }
        else
        {
            enlarge_group(swdiags);
        }

        free(line);
        i++;
    }
    for (;i < 32; i++)
    {
        char name[32];

        sprintf(name, "SWDIAG%d", i);

        /* Add to the SW diags groups */
        add_group(swdiags, name, "sw", i, i, 0);
    }

    fclose(fp);

    return 0;
}

static int create_hwdiag_groups(struct groups *hwdiags, char *path)
{
    int i, j;
    FILE *names;
    FILE *config;
    char hwconfig[2][5] = {"0xXX", "0xYY"};
    char *line;
    ssize_t read;
    char line_prev[256] = "";
    int lsb = 0;
    int diag_cnt = 2;
    int bank_len = 16;
    char hwdiag_name[32] = "hwdiag.txt";
    int read_hw_diags = 1;

    printf("Create HW diag groups\n");

    hwdiags->cnt = 0;

    // First open the platform configuration file to check if the MAC HW or
    // platform diags are output
    config = plfdiag_config_open(path);
    if (config != NULL)
    {
        // Read the platform diag configuration
        read = readline(&line, config);
        if (read != 8)
            return -1;

        if (memcmp("0C", &line[6], 2))
        {
            memcpy(&hwconfig[0][2], &line[6], 2);
            diag_cnt = 1;
            bank_len = 32;
            strcpy(hwdiag_name, "fpgaa_hwdiag.txt");
            read_hw_diags = 0;
        }

        // Free line and close configuration file
        free(line);
        fclose(config);
    }

    if (read_hw_diags)
    {
        config = hwdiag_config_open(path);
        if (config == NULL)
            return -1;

        // Read the HW diag configuration
        read = readline(&line, config);
        if (read != 8)
            return -1;

        // Fill-in the LSB and MSB configurations
        memcpy(&hwconfig[0][2], &line[6], 2);
        memcpy(&hwconfig[1][2], &line[4], 2);

        // Free line and close configuration file
        free(line);
        fclose(config);
    }

    names = hwdiag_name_open(path, hwdiag_name);
    if (names == NULL)
    {
        printf("Unable to create HW diags\n");
        return -1;
    }

    // Walk through the name file and create the groups for the LSB and MSB config
    for (i = 0; i < diag_cnt; i++)
    {
        // Find the beginning of an HW diag configuration in the name file
        while (1)
        {
            // Read the HW diag configuration
            read = readline(&line, names);
            if (read < 0)
                return -1;

            if (!strcmp(line, hwconfig[i]))
            {
                free(line);
                break;
            }

            free(line);
        }

        for (j = 0; j < bank_len; j++)
        {
            read = readline(&line, names);

            if (read <= 0)
                break;

            // Compare with previous diag name to know if it is part of a group
            if (strcmp(line_prev, line))
            {
                /* Create new group */
                add_group(hwdiags, line, "hw", lsb, lsb, 0);
                strcpy(line_prev, line);
            }
            else
            {
                enlarge_group(hwdiags);
            }
            lsb++;
            free(line);
        }
        fseek(names, 0, SEEK_SET);
    }

    fclose(names);

    return 0;
}

static int create_mpif_groups(struct groups *mpif)
{
    printf("Create MPIF groups\n");

    mpif->cnt = 0;

    add_group(mpif, "mpif_rifsDetected",     "mp", 0,  0,  0);
    add_group(mpif, "mpif_keepRfOn",         "mp", 1,  1,  0);
    add_group(mpif, "mpif_phyErr",           "mp", 2,  2,  0);
    add_group(mpif, "mpif_rxEnd_p",          "mp", 3,  3,  0);
    add_group(mpif, "mpif_rxErr_p",          "mp", 4,  4,  0);
    add_group(mpif, "mpif_rxEndForTiming_p", "mp", 5,  5,  0);
    add_group(mpif, "mpif_ccaSecondary",     "mp", 6,  6,  0);
    add_group(mpif, "mpif_ccaPrimary",       "mp", 7,  7,  0);
    add_group(mpif, "mpif_rxReq",            "mp", 16, 16, 0);
    add_group(mpif, "mpif_txEnd_p",          "mp", 17, 17, 0);
    add_group(mpif, "mpif_txReq",            "mp", 28, 28, 0);
    add_group(mpif, "mpif_rfshutdown",       "mp", 29, 29, 0);
    add_group(mpif, "mpif_rxData",           "mp", 8,  15, 0);
    add_group(mpif, "mpif_phyRdy",           "mp", 18, 18, 0);
    add_group(mpif, "mpif_macDataValid",     "mp", 19, 19, 0);
    add_group(mpif, "mpif_txData",           "mp", 20, 27, 0);

    return 0;
}

static int create_control_groups(struct groups *control)
{
    printf("Create control groups\n");

    control->cnt = 0;

    add_group(control, "trigger", "ct", 31,  31,  0);

    return 0;
}

static void declare_groups(FILE *vcd, struct groups *groups)
{
    int i;

    for (i = 0; i < groups->cnt; i++)
    {
        struct group *g = &groups->group[i];

        if (g->lsb == g->msb)
        {
            fprintf(vcd, "$var wire 1 %s %s $end\n", g->id, g->name);
        }
        else
        {
            int width = (g->msb - g->lsb) + 1;
            fprintf(vcd, "$var reg %d %s %s [%d:0] $end\n", width, g->id, g->name, width - 1);
        }
    }
}

static void init_groups(FILE *vcd, struct groups *groups, uint32_t sampling)
{
    int i;

    for (i = 0; i < groups->cnt; i++)
    {
        struct group *g = &groups->group[i];
        int width = (g->msb - g->lsb) + 1;
        uint32_t msk = (((uint32_t)1 << width) - 1);
        uint32_t smp_msk = (sampling >> g->lsb) & msk;

        if (width == 1)
        {
            if (smp_msk)
                fprintf(vcd, "%d%s\n", g->prev, g->id);
            else
                fprintf(vcd, "x%s\n", g->id);
        }
        else
        {
            char bin[64];
            int2bin(bin, g->prev, width, smp_msk);
            fprintf(vcd, "b%s %s\n", bin, g->id);
        }
    }
}


static void put_groups(FILE *vcd, struct groups *groups, uint32_t value, uint32_t sampling)
{
    int i;

    for (i = 0; i < groups->cnt; i++)
    {
        struct group *g = &groups->group[i];
        unsigned int width = (g->msb - g->lsb) + 1;
        unsigned int offset = g->lsb;
        uint32_t msk = (((uint32_t)1 << width) - 1) << offset;
        uint32_t curr = (value & msk) >> offset;
        uint32_t smp_msk = (sampling & msk) >> offset;

        if (curr != g->prev)
        {
            if (width == 1)
            {
                if (smp_msk)
                    fprintf(vcd, "%d%s\n", curr, g->id);
            }
            else
            {
                char bin[64];
                if (int2bin(bin, curr, width, smp_msk) == 0)
                    fprintf(vcd, "b%s %s\n", bin, g->id);
            }
            g->prev = curr;
        }
    }
}

/*
 *****************************************************************************************
 * @brief Main entry point of the application.
 *
 * @param argc   usual parameter counter
 * @param argv   usual parameter values
 *****************************************************************************************
 */
int main(int argc, char **argv)
{
    FILE *vcd;
    FILE *trace;
    uint32_t value[4];
    uint32_t sampling[4];
    uint32_t la_version_mac;
    unsigned long long time = 0;
    unsigned long long delay = DELAY;
    unsigned long long freq;

    uint32_t la_version = 3;

    struct groups swdiags;
    struct groups hwdiags;
    struct groups mpif;
    struct groups controls;

    /* strip off self */
    argc--;
    argv++;

    if (argc == 0)
        return -1;

    /* Get LA configuration */
    get_la_conf(sampling, &la_version_mac, argv[0], "lamacconf");
    sampling[3] = 0xFFFFFFFF;

    /* In case the sampling frequency information is available, recompute the
     * VCD delay
     */
    freq = (unsigned long long)(la_version_mac >> 24);
    if (freq != 0)
    {
        delay = ((unsigned long long)1000000)/freq;
    }

    /* create groups of signals */
    create_swdiag_groups(&swdiags, argv[0]);
    create_hwdiag_groups(&hwdiags, argv[0]);
    create_mpif_groups(&mpif);
    create_control_groups(&controls);

    printf("Create VCD file\n");

    /* create VCD file */
    vcd = vcd_open(argv[0]);
    if (vcd == NULL)
    {
        printf("Failed opening the VCD file\n");
        return -1;
    }

    /* print file header */
    fprintf(vcd, "$comment\nTOOL: RW BLA\n$end\n$date\n$end\n$timescale\n1ps\n$end\n");

    /* declare groups */
    declare_groups(vcd, &controls);
    declare_groups(vcd, &mpif);
    declare_groups(vcd, &swdiags);
    declare_groups(vcd, &hwdiags);

    fprintf(vcd, "\n$enddefinitions\n$dumpvars\n");

    /* initialize groups (first part) */
    init_groups(vcd, &controls, sampling[3]);
    init_groups(vcd, &mpif, sampling[2]);
    init_groups(vcd, &swdiags, sampling[0]);
    init_groups(vcd, &hwdiags, sampling[1]);

    fprintf(vcd, "\n$end\n#0\n");

    /* initialize groups (second part) */
    init_groups(vcd, &controls, sampling[3]);
    init_groups(vcd, &mpif, sampling[2]);
    init_groups(vcd, &swdiags, sampling[0]);
    init_groups(vcd, &hwdiags, sampling[1]);

    /* open trace file from Dini */
    trace = trace_open(argv[0]);
    if (trace == NULL)
    {
        printf("Failed opening the VCD file\n");
        return -1;
    }

    /* go through the whole file */
    while(1)
    {
        int i;
        for (i=0; i<4; i++)
        {
            if (fread(&value[i], 4, 1, trace) != 1)
                break;
        }

        if (i < 4)
            break;

        if (la_version <= 2)
        {
            if (value[3] & 0x80000000)
            {
                time += (unsigned long long)value[0] * DELAY;
            }
            else
            {
                time += delay;

                fprintf(vcd, "#%llu\n", time);

                put_groups(vcd, &controls, value[3], sampling[3]);
                put_groups(vcd, &mpif, value[2], sampling[2]);
                put_groups(vcd, &hwdiags, value[1], sampling[1]);
                put_groups(vcd, &swdiags, value[0], sampling[0]);
            }

        }
        else
        {
            time += (unsigned long long)(value[3] & 0x7FFFFFFF) * delay;

            put_groups(vcd, &controls, value[3] & 0x80000000, sampling[3]);
            put_groups(vcd, &mpif, value[2], sampling[2]);
            put_groups(vcd, &hwdiags, value[1], sampling[1]);
            put_groups(vcd, &swdiags, value[0], sampling[0]);

            fprintf(vcd, "#%llu\n", time);
        }
    }

    fclose(vcd);
    fclose(trace);


    return 0;
}
