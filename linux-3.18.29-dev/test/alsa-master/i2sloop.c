/*
 ** Copyright 2009, Brian Swetland. All rights reserved.
 **
 ** Redistribution and use in source and binary forms, with or without
 ** modification, are permitted provided that the following conditions
 ** are met:
 ** 1. Redistributions of source code must retain the above copyright
 **    notice, this list of conditions, and the following disclaimer.
 ** 2. The name of the authors may not be used to endorse or promote products
 **    derived from this software without specific prior written permission.
 **
 ** THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS OR
 ** IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 ** OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 ** IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 ** INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 ** NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 ** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 ** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 ** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 ** THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include "audio.h"
#define ALSA_PCM_NEW_HW_PARAMS_API
#include <alsa/asoundlib.h>

/* http://ccrma.stanford.edu/courses/422/projects/WaveFormat/ */

#define ID_RIFF 0x46464952
#define ID_WAVE 0x45564157
#define ID_FMT  0x20746d66
#define ID_DATA 0x61746164

#define FORMAT_PCM 1

pthread_t thread[2];
pthread_mutex_t mut;

struct wav_header {
	uint32_t riff_id;
	uint32_t riff_sz;
	uint32_t riff_fmt;
	uint32_t fmt_id;
	uint32_t fmt_sz;
	uint16_t audio_format;
	uint16_t num_channels;
	uint32_t sample_rate;
	uint32_t byte_rate;       /* sample_rate * num_channels * bps / 8 */
	uint16_t block_align;     /* num_channels * bps / 8 */
	uint16_t bits_per_sample;
	uint32_t data_id;
	uint32_t data_sz;
};

int play_file(unsigned rate, unsigned channels, int fd, unsigned count)
{
	struct pcm *pcm;
	unsigned avail, xfer, bufsize;
	char *data, *next;
	int i;
	data = malloc(count);
	if (!data) {
		fprintf(stderr,"could not allocate %d bytes\n", count);
		return -1;
	}
	if (read(fd, data, count) != count) {
		close(fd);
		fprintf(stderr,"could not read %d bytes\n", count);
		return -1;
	}

	close(fd);
	avail = count;
	next = data;
	pcm = pcm_alloc();
	if (pcm_open(pcm))
		goto fail;
	bufsize = pcm_buffer_size(pcm);
	while (avail > 0) {
		xfer = (avail > bufsize) ? bufsize : avail;
		if (pcm_write(pcm, next, xfer))
			goto fail;
		next += xfer;
		avail -= xfer;
	}
	free(data);
	pcm_close(pcm);
	return 0;

fail:
	fprintf(stderr,"pcm error: %s\n", pcm_error(pcm));
	return -1;
}

void *playwav()
{
	sleep(1);
	struct wav_header hdr;
	unsigned rate, channels;
	int fd;
	int i;
	fd = open("/alsa-master1/z.wav", O_RDONLY);
	if (fd < 0) {
		fprintf(stderr, "playwav: cannot open '1.wav'\n");
		return -1;
	}
	if (read(fd, &hdr, sizeof(hdr)) != sizeof(hdr)) {
		fprintf(stderr, "playwav: cannot read header\n");
		return -1;
	}
	fprintf(stderr,"playwav: %d ch, %d hz, %d bit, %s\n",
			hdr.num_channels, hdr.sample_rate, hdr.bits_per_sample,
			hdr.audio_format == FORMAT_PCM ? "PCM" : "unknown");

	if ((hdr.riff_id != ID_RIFF) ||
			(hdr.riff_fmt != ID_WAVE) ||
			(hdr.fmt_id != ID_FMT)) {
		fprintf(stderr, "playwav: '1.wav' is not a riff/wave file\n");
		return -1;
	}
	if ((hdr.audio_format != FORMAT_PCM) ||
			(hdr.fmt_sz != 16)) {
		fprintf(stderr, "playwav: '1.wav' is not pcm format\n");
		return -1;
	}
	if (hdr.bits_per_sample != 16) {
		fprintf(stderr, "playwav: '1.wav' is not 16bit per sample\n");
		return -1;
	}
	return play_file(hdr.sample_rate, hdr.num_channels, fd, hdr.data_sz);
}
void *record() {
	long loops;
	int rc,i = 0;
	int size;
	FILE *fp ;
	snd_pcm_t *handle;
	snd_pcm_hw_params_t *params;
	unsigned int val,val2;
	int dir;
	snd_pcm_uframes_t frames;
	char *buffer;
	if(  (fp =fopen("sound.wav","wb")) < 0)
		printf("open sound.wav fial\n");
	/* Open PCM device for recording (capture). */
	rc = snd_pcm_open(&handle, "default",
			SND_PCM_STREAM_CAPTURE, 0);
	if (rc < 0) {
		fprintf(stderr,  "unable to open pcm device: %s/n",  snd_strerror(rc));
		exit(1);
	}
	/* Allocate a hardware parameters object. */
	snd_pcm_hw_params_alloca(&params);
	/* Fill it in with default values. */
	snd_pcm_hw_params_any(handle, params);
	/* Set the desired hardware parameters. */
	/* Interleaved mode */
	snd_pcm_hw_params_set_access(handle, params,
			SND_PCM_ACCESS_RW_INTERLEAVED);
	/* Signed 16-bit little-endian format */
	snd_pcm_hw_params_set_format(handle, params,
			SND_PCM_FORMAT_S16_LE);
	/* Two channels (stereo) */
	snd_pcm_hw_params_set_channels(handle, params, 2);
	/* 44100 bits/second sampling rate (CD quality) */
	val = 44100;
	snd_pcm_hw_params_set_rate_near(handle, params,  &val, &dir);
	/* Set period size to 32 frames. */
	frames = 2048;
	snd_pcm_hw_params_set_period_size_near(handle,  params, &frames, &dir);
	/* Write the parameters to the driver */
	rc = snd_pcm_hw_params(handle, params);
	if (rc < 0) {
		fprintf(stderr,  "unable to set hw parameters: %s/n",
				snd_strerror(rc));
		exit(1);
	}
	/* Use a buffer large enough to hold one period */
	snd_pcm_hw_params_get_period_size(params,  &frames, &dir);
	size = frames * 4; /* 2 bytes/sample, 2 channels */
	printf("size = %d\n",size);
	buffer = (char *) malloc(size);
	/* We want to loop for 5 seconds */
	snd_pcm_hw_params_get_period_time(params,  &val, &dir);
	printf("val = %d\n",val);
	loops = 30000000 / val;
	while (loops > 0) {
		loops--;
		rc = snd_pcm_readi(handle, buffer, frames);
		if (rc == -EPIPE) {
			/* EPIPE means overrun */
			fprintf(stderr, "overrun occurred\n");
			snd_pcm_prepare(handle);
		} else if (rc < 0) {
			fprintf(stderr,
					"error from read: %s/n",
					snd_strerror(rc));
		} else if (rc != (int)frames) {
			fprintf(stderr, "short read, read %d frames/n", rc);
		}
		rc = fwrite( buffer,1, size,fp);
		//      rc = write(1,buffer,size);
		if (rc != size)
			fprintf(stderr,  "short write: wrote %d bytes/n", rc);
		//		else printf("fwrite buffer success\n");
	}
	/*******************************************************************/
	snd_pcm_drain(handle);
	snd_pcm_close(handle);
	fclose(fp);
	free(buffer);
	return 0;
}



void thread_create()
{
	int temp;
	memset(&thread, 0, sizeof(thread));          //comment1
	/*创建线程*/
	if((temp = pthread_create(&thread[0], NULL, record, NULL)) != 0)       //comment2
		printf("err creat pthread record\n");
	else
		printf("creat pthread record\n");
	if((temp = pthread_create(&thread[1], NULL, playwav, NULL)) != 0)  //comment3
		printf("err creat pthread playwav\n");
	else
		printf("creat pthread playwav\n");
}

pthread_mutex_t mut;

void thread_wait(void)
{
	/*等待线程结束*/
	if(thread[0] !=0) {                   //comment4
		pthread_join(thread[0],NULL);
		printf("pthread record end\n");
	}
	if(thread[1] !=0) {                //comment5
		pthread_join(thread[1],NULL);
		printf("pthread playwav end\n");
	}
}

//remove initial sound's head
int rmhead()
{
	int bytes,bytes1;
	int t,s;    
	char* buf[1024] = {0};
	int fd4 = open("/alsa-master1/z.wav",O_RDWR);
	int fd1 = open("/alsa-master1/1.wav",O_RDWR);
	lseek(fd4,44,SEEK_SET);
	for(t = 1; t >0; t++) {
		if(bytes = read(fd4,buf,1024) == 1024){
			bytes1 = write(fd1,buf,1024);
		}
		else {
			bytes1 = write(fd1,buf,bytes);
			t = -1;
		}
	}
	close(fd4);
	close(fd1);
}

//compare inictial data and record data
int compare(int t)
{
	int bytes,bytes1,s;
	char buf1[1024] = {0};
	char buf2[1024] = {0};
	int fd2 = open("/alsa-master1/1.wav",O_RDWR);
	int fd3 = open("/alsa-master1/sound.wav",O_RDWR);
	for(s = 1; s > 0; s++) {
		printf("s = %d\n",s);
		bytes = read(fd2, buf1, 1024);
		if( bytes == 1024) {
			printf("read success\n");
			bytes1 = read(fd3, buf2, 1024);
		}
		else {
			bytes1 = read(fd3, buf2, bytes);
			printf("read end\n");
			s = -1;
		}
		int i = memcmp(buf1, buf2 ,bytes);
		if(i == 0){
			printf("buf1 = buf2\n");
		}
		else {
			char buff[4] ={'0','0','0', '0'};

			if(memcmp(buff,buf2,2)) {
				lseek(fd3,2,SEEK_SET);
				lseek(fd2,-1024,SEEK_SET);
				lseek(fd3,-1024,SEEK_SET);

				continue;
			}
			t++;
			printf("t = %d\n",t);
			printf("buf1 != buf2\n");
			printf("buf1 = ");
			for(i = 0; i < 1024; i++)
				printf("%02x",buf1[i]);
			printf("\n");
			printf("buf2 = ");
			for(i = 0; i < 1024; i++)
				printf("%02x",buf2[i]);
			printf("\n");
		}
	}
	return t;
}
int main()
{
	//write z.wav to 1.wav and remove file head 

	rmhead();
	pthread_mutex_init(&mut,NULL);
	int k,j = 0;
	for(k = 0; k < 100; k++) {
		//	printf("----------------- k = %d\n",k);
		thread_create();
		thread_wait();
		j = compare(j);
		printf("j = %d\n",j);
	}
	printf("j = %d\n",j);
	if(j == 0)
		printf("compare success\n");
	else 
		printf("compare fail\n");
	return 0;
}
