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
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * DEFINES
 *****************************************************************************************
 */
struct rx_hd
{
    uint32_t upatternrx;
    uint32_t next;
    uint32_t first_pbd_ptr;
    uint32_t swdesc;
    uint32_t datastartptr;
    uint32_t dataendptr;
    uint32_t headerctrlinfo;
    uint32_t frmlen;
    uint32_t tsflo;
    uint32_t tsfhi;
    uint32_t recvec1a;
    uint32_t recvec1b;
    uint32_t recvec1c;
    uint32_t recvec1d;
    uint32_t recvec2a;
    uint32_t recvec2b;
    uint32_t statinfo;
};

struct rx_bd
{
    uint32_t upattern;
    uint32_t next;
    uint32_t datastartptr;
    uint32_t dataendptr;
    uint16_t bufstatinfo;
    uint16_t reserved;
};

struct tx_hd
{
    uint32_t upatterntx;
    uint32_t nextfrmexseq_ptr;
    uint32_t nextmpdudesc_ptr;
    uint32_t first_pbd_ptr;
    uint32_t datastartptr;
    uint32_t dataendptr;
    uint32_t frmlen;
    uint32_t frmlifetime;
    uint32_t phyctrlinfo;
    uint32_t policyentryaddr;
    uint32_t opt20mhzlen;
    uint32_t opt40mhzlen;
    uint32_t opt80mhzlen;
    uint32_t macctrlinfo1;
    uint32_t macctrlinfo2;
    uint32_t statinfo;
    uint32_t mediumtimeused;
};

/// Definition of a TX payload buffer descriptor.
struct tx_pbd
{
    uint32_t upatterntx;
    uint32_t next;
    uint32_t datastartptr;
    uint32_t dataendptr;
    uint32_t bufctrlinfo;
};

struct tx_policy_tbl
{
    uint32_t upatterntx;
    uint32_t phycntrlinfo1;
    uint32_t phycntrlinfo2;
    uint32_t maccntrlinfo1;
    uint32_t maccntrlinfo2;
    uint32_t ratecntrlinfo[4];
    uint32_t powercntrlinfo[4];
};

/*
 * FUNCTION DEFINITIONS
 *****************************************************************************************
 */
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

static int get_rxdesc_pointers(char *path, char **rhd, char **rbd, uint32_t *hwrhd, uint32_t *hwrbd)
{
    FILE *fp;
    char filename[256];
    ssize_t read;

    printf("Get RX descriptor pointers\n");

    snprintf(filename, 255, "%s/rxdesc", path);
    fp = fopen(filename, "r");
    if (fp == NULL)
        return -1;

    // Read the RX descriptor pointers
    read = readline(rhd, fp);
    if (read != 8)
        return -1;

    read = readline(rbd, fp);
    if (read != 8)
        return -1;

    fclose(fp);

    snprintf(filename, 255, "%s/macrxptr", path);
    fp = fopen(filename, "r");
    if (fp == NULL)
        return 0;

    // Read the RX descriptor pointers
    do
    {
        if (fread(hwrhd, sizeof(*hwrhd), 1, fp) != 1)
        {
            *hwrhd = 0xFFFFFFFF;
            break;
        }

        if (fread(hwrbd, sizeof(*hwrbd), 1, fp) != 1)
        {
            *hwrbd = 0xFFFFFFFF;
            break;
        }
    } while(0);

    fclose(fp);

    return 0;
}

static int get_txdesc_pointers(char *path, char *thd[])
{
    FILE *fp;
    char filename[256];
    ssize_t read;
    int i;

    printf("Get TX descriptor pointers\n");

    snprintf(filename, 255, "%s/txdesc", path);
    fp = fopen(filename, "r");
    if (fp == NULL)
        return -1;

    // Read the TX descriptor pointers
    for (i = 0; i < 5; i++)
    {
        read = readline(&thd[i], fp);
        if (read != 8)
            break;
    }

    fclose(fp);

    return i;
}

static int gen_rhd(char *path, char *first, uint32_t hwrhd)
{
    FILE *in;
    FILE *out;
    char filename[256];
    int i = 1;
    struct rx_hd rhd;
    uint32_t ptr;
    bool hwrhd_found = false;

    if (sscanf(first, "%08x", &ptr) != 1)
        ptr = 0;

    printf("Generate RHD file\n");

    snprintf(filename, 255, "%s/rhd", path);
    in = fopen(filename, "r");

    if (in == NULL)
        return -1;

    snprintf(filename, 255, "%s/rhd.txt", path);
    out = fopen(filename, "w");

    if (out == NULL)
        return -1;

    fprintf(out, "RX Header Descriptor list\n\n");

    while (1)
    {
        if (fread(&rhd, sizeof(rhd), 1, in) != 1)
            break;
        fprintf(out, "RHD#%d @ 0x%08x", i, ptr);

        if (hwrhd != 0xFFFFFFFF)
        {
            if (i == 1)
                fprintf(out, "    <--- Spare RHD");
            else if (i == 2)
                fprintf(out, "    <--- First RHD for SW");

            if (ptr == hwrhd)
            {
                hwrhd_found = true;
                if (i < 3)
                    fprintf(out, " - Current RHD for HW");
                else
                    fprintf(out, "    <--- Current RHD for HW");
            }
        }

        fprintf(out, "\n~~~~~~~~~~~~~~~~~~~\n");
        if (rhd.upatternrx != 0xBAADF00D)
            fprintf(out, "UNIQUE PATTERN ERROR!!!!\n");

        fprintf(out, "  Unique pattern                 = 0x%08x\n", rhd.upatternrx);
        fprintf(out, "  Next Header Descriptor Pointer = 0x%08x\n", rhd.next);
        fprintf(out, "  First Payload Buffer Pointer   = 0x%08x\n", rhd.first_pbd_ptr);
        fprintf(out, "  Software Descriptor Pointer    = 0x%08x\n", rhd.swdesc);
        fprintf(out, "  Data Start Pointer             = 0x%08x\n", rhd.datastartptr);
        fprintf(out, "  Data End Pointer               = 0x%08x\n", rhd.dataendptr);
        fprintf(out, "  Header Control Info            = 0x%08x\n", rhd.headerctrlinfo);
        fprintf(out, "  Frame Length                   = 0x%08x\n", rhd.frmlen);
        fprintf(out, "  TSF Low                        = 0x%08x\n", rhd.tsflo);
        fprintf(out, "  TSF High                       = 0x%08x\n", rhd.tsfhi);
        fprintf(out, "  Rx Vector 1a                   = 0x%08x\n", rhd.recvec1a);
        fprintf(out, "  Rx Vector 1b                   = 0x%08x\n", rhd.recvec1b);
        fprintf(out, "  Rx Vector 1c                   = 0x%08x\n", rhd.recvec1c);
        fprintf(out, "  Rx Vector 1d                   = 0x%08x\n", rhd.recvec1d);
        fprintf(out, "  Rx Vector 2a                   = 0x%08x\n", rhd.recvec1d);
        fprintf(out, "  Rx Vector 2b                   = 0x%08x\n", rhd.recvec2a);
        fprintf(out, "  Status Information             = 0x%08x\n\n\n", rhd.statinfo);

        ptr = rhd.next;

        i++;
    }

    fprintf(out, "\nEnd of list\n");
    if ((hwrhd != 0xFFFFFFFF) && !hwrhd_found)
        fprintf(out, "\n!!!! MAC HW is pointing to unknown RHD@0x%08X !!!!\n", hwrhd);

    fclose(in);
    fclose(out);

    return 0;
}

static int gen_rbd(char *path, char *first, uint32_t hwrbd)
{
    FILE *in;
    FILE *out;
    char filename[256];
    int i = 1;
    struct rx_bd rbd;
    uint32_t ptr;
    bool hwrbd_found = false;

    if (sscanf(first, "%08x", &ptr) != 1)
        ptr = 0;

    printf("Generate RBD file\n");

    snprintf(filename, 255, "%s/rbd", path);
    in = fopen(filename, "r");

    if (in == NULL)
        return -1;

    snprintf(filename, 255, "%s/rbd.txt", path);
    out = fopen(filename, "w");

    if (out == NULL)
        return -1;

    fprintf(out, "RX Buffer Descriptor list\n\n");

    while (1)
    {
        if (fread(&rbd, sizeof(rbd), 1, in) != 1)
            break;
        fprintf(out, "RBD#%d @ 0x%08x", i, ptr);

        if (hwrbd != 0xFFFFFFFF)
        {
            if (i == 1)
                fprintf(out, "    <--- Spare RBD");
            else if (i == 2)
                fprintf(out, "    <--- First RBD for SW");

            if (ptr == hwrbd)
            {
                hwrbd_found = true;
                if (i < 3)
                    fprintf(out, " - Current RBD for HW");
                else
                    fprintf(out, "    <--- Current RBD for HW");
            }
        }

        fprintf(out, "\n~~~~~~~~~~~~~~~~~~~\n");
        if (rbd.upattern != 0xC0DEDBAD)
            fprintf(out, "UNIQUE PATTERN ERROR!!!!\n");

        fprintf(out, "  Unique pattern                 = 0x%08x\n", rbd.upattern);
        fprintf(out, "  Next Buffer Descriptor Pointer = 0x%08x\n", rbd.next);
        fprintf(out, "  Data Start Pointer             = 0x%08x\n", rbd.datastartptr);
        fprintf(out, "  Data End Pointer               = 0x%08x\n", rbd.dataendptr);
        fprintf(out, "  Buffer Status Info             = 0x%08x\n\n\n", rbd.bufstatinfo);

        ptr = rbd.next;

        i++;
    }

    fprintf(out, "\nEnd of list\n");
    if ((hwrbd != 0xFFFFFFFF) && !hwrbd_found)
        fprintf(out, "\n!!!! MAC HW is pointing to unknown RBD@0x%08X !!!!\n", hwrbd);

    fclose(in);
    fclose(out);

    return 0;
}

static int gen_thd(char *path, char *first, int idx)
{
    FILE *in;
    FILE *out;
    char filename[256];
    int i = 1;
    struct tx_hd thd;
    struct tx_pbd tbd;
    struct tx_policy_tbl pol;
    uint32_t tbd_ptr;

    printf("Generate THD file %d\n", idx);

    snprintf(filename, 255, "%s/thd%d", path, idx);
    in = fopen(filename, "r");

    if (in == NULL)
        return -1;

    snprintf(filename, 255, "%s/thd%d.txt", path, idx);
    out = fopen(filename, "w");

    if (out == NULL)
        return -1;

    fprintf(out, "TX Header Head Pointer = 0x%s\n\n", first);

    while (1)
    {
        if (fread(&thd, sizeof(thd), 1, in) != 1)
            break;

        fprintf(out, "TX Header Descriptor n=%d\n", i);
        fprintf(out, "~~~~~~~~~~~~~~~~~~~~~~~~~\n");
        if (thd.upatterntx != 0xCAFEBABE)
            fprintf(out, "UNIQUE PATTERN ERROR!!!!\n");

        fprintf(out, "  Unique pattern               = 0x%08x\n", thd.upatterntx);
        fprintf(out, "  Next Atomic Pointer          = 0x%08x\n", thd.nextfrmexseq_ptr);
        fprintf(out, "  Next MPDU Descriptor Pointer = 0x%08x\n", thd.nextmpdudesc_ptr);
        fprintf(out, "  First Payload Buffer Pointer = 0x%08x\n", thd.first_pbd_ptr);
        fprintf(out, "  Data Start Pointer           = 0x%08x\n", thd.datastartptr);
        fprintf(out, "  Data End Pointer             = 0x%08x\n", thd.dataendptr);
        fprintf(out, "  Frame Length                 = 0x%08x\n", thd.frmlen);
        fprintf(out, "  Frame Lifetime               = 0x%08x\n", thd.frmlifetime);
        fprintf(out, "  Phy Control Info             = 0x%08x\n", thd.phyctrlinfo);
        fprintf(out, "  Policy Entry Address         = 0x%08x\n", thd.policyentryaddr);
        fprintf(out, "  Optional 20MHz Length        = 0x%08x\n", thd.opt20mhzlen);
        fprintf(out, "  Optional 40MHz Length        = 0x%08x\n", thd.opt40mhzlen);
        fprintf(out, "  Optional 80MHz Length        = 0x%08x\n", thd.opt80mhzlen);
        fprintf(out, "  MAC Control Information 1    = 0x%08x\n", thd.macctrlinfo1);
        fprintf(out, "  MAC Control Information 2    = 0x%08x\n", thd.macctrlinfo2);
        fprintf(out, "  Status Information           = 0x%08x\n", thd.statinfo);
        fprintf(out, "  Medium Time Used             = 0x%08x\n\n", thd.mediumtimeused);

        // Check if we have to display the policy table
        if (!((thd.macctrlinfo2 & 0x00200000) &&
             (thd.macctrlinfo2 & 0x00180000)))
        {
            if (fread(&pol, sizeof(pol), 1, in) != 1)
                break;

            fprintf(out, "  Policy table\n");
            fprintf(out, "  ~~~~~~~~~~~~\n");
            if (pol.upatterntx != 0xBADCAB1E)
                fprintf(out, "UNIQUE PATTERN ERROR!!!!\n");

            fprintf(out, "    Unique pattern                      = 0x%08x\n", pol.upatterntx);
            fprintf(out, "    P-Table PHY Control Info 1          = 0x%08x\n", pol.phycntrlinfo1);
            fprintf(out, "    P-Table PHY Control Info 2          = 0x%08x\n", pol.phycntrlinfo2);
            fprintf(out, "    P-Table MAC Control Info 1          = 0x%08x\n", pol.maccntrlinfo1);
            fprintf(out, "    P-Table MAC Control Info 2          = 0x%08x\n", pol.maccntrlinfo2);
            fprintf(out, "    P-Table Rate Control Information 1  = 0x%08x\n", pol.ratecntrlinfo[0]);
            fprintf(out, "    P-Table Rate Control Information 2  = 0x%08x\n", pol.ratecntrlinfo[1]);
            fprintf(out, "    P-Table Rate Control Information 3  = 0x%08x\n", pol.ratecntrlinfo[2]);
            fprintf(out, "    P-Table Rate Control Information 4  = 0x%08x\n", pol.ratecntrlinfo[3]);
            fprintf(out, "    P-Table Power Control Information 1 = 0x%08x\n", pol.powercntrlinfo[0]);
            fprintf(out, "    P-Table Power Control Information 2 = 0x%08x\n", pol.powercntrlinfo[1]);
            fprintf(out, "    P-Table Power Control Information 3 = 0x%08x\n", pol.powercntrlinfo[2]);
            fprintf(out, "    P-Table Power Control Information 4 = 0x%08x\n\n", pol.powercntrlinfo[3]);
        }

        // Display the list of TBD attached to this THD
        tbd_ptr = thd.first_pbd_ptr;
        while (tbd_ptr != 0)
        {
            if (fread(&tbd, sizeof(tbd), 1, in) != 1)
                break;

            fprintf(out, "  TX Buffer Descriptor\n");
            fprintf(out, "  ~~~~~~~~~~~~~~~~~~~~\n");
            if (tbd.upatterntx != 0xCAFEFADE)
                fprintf(out, "UNIQUE PATTERN ERROR!!!!\n");

            fprintf(out, "    Unique pattern               = 0x%08x\n", tbd.upatterntx);
            fprintf(out, "    Next Buf Descriptor Pointer  = 0x%08x\n", tbd.next);
            fprintf(out, "    Data Start Pointer           = 0x%08x\n", tbd.datastartptr);
            fprintf(out, "    Data End Pointer             = 0x%08x\n", tbd.dataendptr);
            fprintf(out, "    Buffer Control Information   = 0x%08x\n\n", tbd.bufctrlinfo);

            if (tbd.upatterntx != 0xCAFEFADE)
                break;

            tbd_ptr = tbd.next;
        }
        fprintf(out, "\n");

        i++;
    }

    fprintf(out, "\nEnd of list\n");

    fclose(in);
    fclose(out);

    return 0;
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
    char *first_rhd, *first_rbd;
    uint32_t cur_rhd = 0xFFFFFFFF, cur_rbd = 0xFFFFFFFF;
    char *thd[5];
    int txcnt;
    int i;

    /* strip off self */
    argc--;
    argv++;

    if (argc == 0)
        return -1;

    if (get_rxdesc_pointers(argv[0], &first_rhd, &first_rbd, &cur_rhd, &cur_rbd))
        return -1;

    txcnt = get_txdesc_pointers(argv[0], thd);
    if (txcnt < 0)
        return -1;

    if (gen_rhd(argv[0], first_rhd, cur_rhd))
        return -1;

    if (gen_rbd(argv[0], first_rbd, cur_rbd))
        return -1;

    for (i = 0; i < txcnt; i++)
    {
        if (gen_thd(argv[0], thd[i], i))
            return -1;
    }

    free(first_rhd);
    free(first_rbd);

    return 0;
}
