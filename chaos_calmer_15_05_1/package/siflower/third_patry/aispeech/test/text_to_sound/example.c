/*
	example: 基础技术语音合成

	配置：
	productId：dui平台上基础技术－语音合成的产品id
	aliasKey: 此项不设置（不能设置为NULL)
	asrRes: 此项不设置（不能设置为NULL)

	做合成时，发送DDS_EV_IN_CUSTOM_TTS_TEXT事件给dds
	合成的音频链接通过回调中的DDS_EV_OUT_TTS抛出

	struct dds_msg *msg = dds_msg_new();
	dds_msg_set_type(msg, DDS_EV_IN_CUSTOM_TTS_TEXT);
	dds_msg_set_string(msg, "text", "今天天气非常好");
	dds_send(msg);
	dds_msg_delete(msg);
	
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "dds.h"

static const char *productId = "278575522";
static const char *aliasKey = "1";
static const char *deviceProfile = "/Xuqo/dvSlxi/HdzDZ+V3LtNxhVGZyPFKfzcuZXzfMybuP5BLmBfXITdm5qJlpyarJqcjZqL3cXdz5ucyMmbzceZm8jGy8jHycbOy5rGmcqcy8/LxprIncrd092ek5OQiN3FztPdj42Qm4qci7ab3cXdzcjHysjKys3N3dPdm5qJlpyasZ6Smt3F3c+cm8vNyc+cys/Iz8vNzc7HmszMysuZxpvNnM3NyJrH3dPdjJyQj5rdxaTdnpOT3aKC";
static const char *savedProfile = "/lib/firmware/278575522.txt";
static const char *productKey = "xxx";
static const char *productSecret = "yyy";

enum _status{
	DDS_STATUS_NONE = 0,
	DDS_STATUS_IDLE,
	DDS_STATUS_LISTENING,
	DDS_STATUS_UNDERSTANDING
};

enum _status dds_status;
static int is_get_asr_result = 0;
static int is_get_tts_url = 0;
static int is_dui_response = 0;

static int dds_ev_ccb(void *userdata, struct dds_msg *msg) {
	int type;
	if (!dds_msg_get_type(msg, &type)) {
		switch (type) {
		case DDS_EV_OUT_STATUS: {
			char *value;
			if (!dds_msg_get_string(msg, "status", &value)) {
				printf("dds cur status: %s\n", value);
				if (!strcmp(value, "idle")) {
					dds_status = DDS_STATUS_IDLE;
				} else if (!strcmp(value, "listening")) {
					dds_status = DDS_STATUS_LISTENING;
				} else if (!strcmp(value, "understanding")) {
					dds_status = DDS_STATUS_UNDERSTANDING;
				}
			}
			break;
		}
		case DDS_EV_OUT_CINFO_RESULT: {
			char *value;
			if (!dds_msg_get_string(msg, "result", &value)) {
				printf("result: %s\n", value);
			}
			if (!dds_msg_get_string(msg, "cinfo", &value)) {
				printf("cinfo: %s\n", value);
			}
			break;
		}
		case DDS_EV_OUT_ASR_RESULT: {
			char *value;
			if (!dds_msg_get_string(msg, "var", &value)) {
				printf("var: %s\n", value);
			}
			if (!dds_msg_get_string(msg, "text", &value)) {
				printf("text: %s\n", value);
				is_get_asr_result = 1;
			}
			break;
		}
		case DDS_EV_OUT_TTS: {
			char *value;
			if (!dds_msg_get_string(msg, "speakUrl", &value)) {
				printf("speakUrl: %s\n", value);
				is_get_tts_url = 1;
			}
			break;
		}
		case DDS_EV_OUT_DUI_RESPONSE: {
            char *resp = NULL;
            if(!dds_msg_get_string(msg, "response", &resp)) {
            	printf("dui response: %s\n", resp);
            }
            is_dui_response = 1;
            break;
        }
		case DDS_EV_OUT_ERROR: {
			char *value;
			if (!dds_msg_get_string(msg, "error", &value)) {
				printf("DDS_EV_OUT_ERROR: %s\n", value);
			}
			is_dui_response = 1;
			break;
		}
		default:
			break;
		}
	}
	return 0;
}


void *_run(void *arg) {
	struct dds_msg *msg = dds_msg_new();
	dds_msg_set_string(msg, "productId", productId);
	dds_msg_set_string(msg, "aliasKey", aliasKey);
	dds_msg_set_string(msg, "deviceProfile", deviceProfile);

	struct dds_opt opt;
	opt._handler = dds_ev_ccb;
	opt.userdata = arg;
	dds_start(msg, &opt);
	dds_msg_delete(msg);

	return NULL;
}

void send_request(char *str) {
	struct dds_msg *msg = NULL;

	msg = dds_msg_new();
	dds_msg_set_type(msg, DDS_EV_IN_CUSTOM_TTS_TEXT);
	dds_msg_set_string(msg, "text", str);
	dds_msg_set_string(msg, "voiceId", "zhilingfa");
	dds_msg_set_double(msg, "speed", 1.1);
	dds_msg_set_integer(msg, "volume", 0.7);
	dds_send(msg);
	dds_msg_delete(msg);
	msg = NULL;

	while (1) {
		if (is_get_tts_url || is_dui_response) break;
		usleep(10000);
	}

}

int main(int argc, char **argv) {
	struct dds_msg *msg = NULL;

	pthread_t tid;
	pthread_create(&tid, NULL, _run, NULL);

	while (1) {
		if (dds_status == DDS_STATUS_IDLE) break;
		usleep(10000);
	}

	send_request(argv[1]);

	msg = dds_msg_new();
	dds_msg_set_type(msg, DDS_EV_IN_EXIT);
	dds_send(msg);
	dds_msg_delete(msg);


	pthread_join(tid, NULL);

	return 0;
}
