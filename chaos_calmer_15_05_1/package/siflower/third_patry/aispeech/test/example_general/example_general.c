#include <stdio.h>
#include <string.h>

static const char *productId = "278575522";
static const char *aliasKey = "205ec6a273654205b65a360c48abb8e3";
static const char *deviceProfile = "Nk7yNJ5Z4CzMue3w0zHF8XOHlYjPULbQYc3IMohL+xsp333+/FtxtITdm5qJlpyarJqcjZqL3cXdyJmcysmex83Ly8fGy8ucmZ6cx87Lz86eyJyaxs7Lycrd092ek5OQiN3FztPdj42Qm4qci7ab3cXdzcjHysjKys3N3dPdm5qJlpyasZ6Smt3F3c+czcbIm57Px83GxsudzJmey8qcysaenp6eyp3KyMac3dPdjJyQj5rdxaTdnpOT3aKC";
static const char *savedProfile = "example_general/100002151.txt";
static const char *productKey = "xxx";
static const char *productSecret = "yyy";
static FILE *out_file;

enum _status{
	DDS_STATUS_NONE = 0,
	DDS_STATUS_IDLE,
	DDS_STATUS_LISTENING,
	DDS_STATUS_UNDERSTANDING
};

enum _status dds_status;


int write_dui_response(char *resp)
{
    int len;
    int tmp;
    int write_size;
    char *data;
    if (!resp)
        return -1;
    out_file = fopen("/tmp/aispeech_out.txt", "wb");
    if (!out_file) {
        printf("can not open /tmp/aispeech_out.txt\n");
        return -2;
    }
    len = strlen(resp);
    data = resp;
    tmp = 0;
    write_size = 0;
    while (1) {
        write_size = fwrite(data + tmp, 1, len - tmp, out_file);
        tmp += write_size;
        if (tmp >= len)
            break;
    }
    fclose(out_file);
    return 0;
}
