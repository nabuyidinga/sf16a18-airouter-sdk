/*
 * =====================================================================================
 *
 *       Filename:  gdu_test.c
 *
 *    Description:  gdu simple test
 *
 *        Version:  1.0
 *        Created:  2017年08月15日 15时24分36秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  qi , qi.zhang@siflower.com.cn
 *        Company:  Siflower Communication Tenology Co.,Ltd
 *
 * =====================================================================================
 */
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <memory.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <video/sfax8fb.h>

static char *dev0="/dev/fb0";
static char *dev1="/dev/fb1";

struct _fb_info {
	int fd;
	void *screen_base;
	size_t screen_size;
	__u32 width;
	__u32 height;
	__u32 line_length;
	__u32 bpp;
};

static void fill_rectangle(struct _fb_info *info, struct fb_fillrect *rect)
{
	unsigned long width = rect->width;
	unsigned long height = rect->height;
	unsigned long *dst, *p;
	int i = 0, j = 0;

	if ((rect->dx > info->width) || (rect->dy > info->height))
		return;

	if ((rect->dx + rect->width) > info->width)
		rect->width = info->width - rect->dx;
	if ((rect->dy + rect->height) > info->height)
		rect->height = info->height - rect->dy;

	dst = info->screen_base + info->line_length * rect->dy + 4 * rect->dx;

	for (i = 0; i < rect->height; i++) {
		for (j = 0, p = dst; j < rect->width; j++)
			*p++ = rect->color;
		dst += info->width;
	}
	//msync(info->screen_base, info->screen_size, MS_SYNC);
}

static int random_rectangle(struct _fb_info *info)
{
	struct fb_fillrect rect;
	__u32 width = info->width;
	__u32 height = info->height;

	rect.dx = rand() % width;
	rect.dy = rand() % height;
	rect.width = rand() % width;
	rect.height = rand() % height;
	rect.color = rand() & 0xffffff;

	printf("fill rectangle: %d - %d (%d X %d) by color %#x\n",
	    rect.dx, rect.dy, rect.width, rect.height, rect.color);
	fill_rectangle(info, &rect);

	return 0;
}

static int random_display(struct _fb_info *info, struct fb_var_screeninfo var)
{
	__u32 width = info->width;
	__u32 height = info->height;

	var.xres = rand() % width;
	var.yres = rand() % height;
	var.xoffset = rand() % (width - var.xres);
	var.yoffset = rand() % (height - var.yres);

	printf("set screen %d X %d, offset (%d, %d)\n", var.xres,
	    var.yres, var.xoffset, var.yoffset);
	if (ioctl(info->fd, FBIOPUT_VSCREENINFO, &var)) {
		printf("set screen failed\n");
		return -1;
	}
	return 0;
}

static int random_alpha(int file)
{
	struct sfax8_fb_alpha alpha = {
		.blend_category = per_plane,
		.alpha_sel = USING_ALPHA_1,
	};

	alpha.alpha_1 = rand() & 0xfff;

	printf("set alpha %#x\n", alpha.alpha_0);
	if (ioctl(file, SFFB_PUT_ALPHA, &alpha)) {
		printf("set alpha failed\n");
		return -1;
	}
	return 0;
}

static int get_fb_device(char *dev, struct _fb_info *info)
{
	struct fb_var_screeninfo var;
	struct fb_fix_screeninfo fix;
	int fb = open(dev, O_RDWR);
	size_t size;

	if (fb < 0) {
		printf("can't open device\n");
		return -1;
	}
	info->fd = fb;

	if (ioctl(fb, FBIOGET_VSCREENINFO, &var)) {
		printf("can't get var info of fb0\n");
		goto err;
	}
	info->width = var.xres;
	info->height = var.yres;
	info->bpp = var.bits_per_pixel;

	if (ioctl(fb, FBIOGET_FSCREENINFO, &fix)) {
		printf("can't get fix info of fb0\n");
		goto err;
	}
	info->line_length = fix.line_length;

	info->screen_size = info->line_length * info->height;
	info->screen_base = mmap(0, info->screen_size, PROT_READ | PROT_WRITE, MAP_SHARED, fb, 0);
	if (info->screen_base == MAP_FAILED) {
		printf("map failed\n");
		goto err;
	}
	printf("got %s's base: %p, size %d X %d\n", dev, info->screen_base,
	    info->width, info->height);
	return 0;

err:
	close(fb);
	memset(info, sizeof(*info), 0);
	return -1;
}

static void put_fb_device(struct _fb_info *info)
{
	munmap(info->screen_base, info->line_length * info->height);
	close(info->fd);
}

int main()
{
	struct _fb_info info0, info1;
	struct fb_var_screeninfo var1;

	if (get_fb_device(dev0, &info0)) {
		printf("get fb0 failed\n");
		return -1;
	}
	/*if (get_fb_device(dev1, &info1)) {
		printf("get fb1 failed\n");
		goto err_fb1;
	}

	if (ioctl(info1.fd, FBIOGET_VSCREENINFO, &var1)) {
		printf("can't get var info of fb0\n");
		goto err;
	}*/

	do {
		if (random_rectangle(&info0))
			break;
		/* if (random_display(&info1, var1))
			break;
		if (random_alpha(info1.fd))
			break;
		mdelay(500);*/
	} while (1);

err:
//	put_fb_device(&info1);
err_fb1:
	put_fb_device(&info0);
	return -1;
}
