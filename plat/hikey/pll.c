/*
 * Copyright (c) 2014-2015, Linaro Ltd and Contributors. All rights reserved.
 * Copyright (c) 2014-2015, Hisilicon Ltd and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <arch_helpers.h>
#include <console.h>
#include <debug.h>
#include <errno.h>
#include <hi6220.h>
#include <hi6553.h>
#include <mmio.h>
#include <platform_def.h>
#include <sp804_timer.h>

//#define DDR800

static void init_pll(void)
{
	unsigned int data;

	dsb();
	data = mmio_read_32((0xf7032000 + 0x000));
	data |= 0x1;
	mmio_write_32((0xf7032000 + 0x000), data);
	dsb();
	do {
		data = mmio_read_32((0xf7032000 + 0x000));
	} while (!(data & (1 << 28)));

	data = mmio_read_32((0xf7800000 + 0x000));
	data &= ~0x007;
	data |= 0x004;
	mmio_write_32((0xf7800000 + 0x000), data);
	dsb();
	do {
		data = mmio_read_32((0xf7800000 + 0x014));
		data &= 0x007;
	} while (data != 0x004);
	dsb();
}

static void init_freq(void)
{
	unsigned int data, tmp;
	unsigned int cpuext_cfg, ddr_cfg;

	mmio_write_32((0xf7032000 + 0x374), 0x4a);
	mmio_write_32((0xf7032000 + 0x368), 0xda);
	mmio_write_32((0xf7032000 + 0x36c), 0x01);
	mmio_write_32((0xf7032000 + 0x370), 0x01);
	mmio_write_32((0xf7032000 + 0x360), 0x60);
	mmio_write_32((0xf7032000 + 0x364), 0x60);

	mmio_write_32((0xf7032000 + 0x114), 0x1000);

	data = mmio_read_32((0xf7032000 + 0x110));
	data |= (3 << 12);
	mmio_write_32((0xf7032000 + 0x110), data);

	data = mmio_read_32((0xf7032000 + 0x110));
	data |= (1 << 4);
	mmio_write_32((0xf7032000 + 0x110), data);


	data = mmio_read_32((0xf7032000 + 0x110));
	data &= ~0x7;
	data |= 0x5;
	mmio_write_32((0xf7032000 + 0x110), data);
	dsb();
	mdelay(10);


	do {
		data = mmio_read_32((0xf6504000 + 0x008));
		data &= (3 << 20);
	} while (data != (3 << 20));
	dsb();
	mdelay(10);

	data = mmio_read_32((0xf6504000 + 0x054));
	data &= ~((1 << 0) | (1 << 11));
	mmio_write_32((0xf6504000 + 0x054), data);
	dsb();
	mdelay(10);

	data = mmio_read_32((0xf7032000 + 0x104));
	data &= ~(3 << 8);
	data |= (1 << 8);
	mmio_write_32((0xf7032000 + 0x104), data);

	data = mmio_read_32((0xf7032000 + 0x100));
	data |= (1 << 0);
	mmio_write_32((0xf7032000 + 0x100), data);
	dsb();

	do {
		data = mmio_read_32((0xf7032000 + 0x100));
		data &= (1 << 2);
	} while (data != (1 << 2));

	dsb();
	data = mmio_read_32((0xf6504000 + 0x06c));
	data &= ~0xffff;
	data |= 0x56;
	mmio_write_32((0xf6504000 + 0x06c), data);

	data = mmio_read_32((0xf6504000 + 0x06c));
	data &= ~(0xffffff << 8);
	data |= 0xc7a << 8;
	mmio_write_32((0xf6504000 + 0x06c), data);

	data = mmio_read_32((0xf6504000 + 0x058));
	data &= ((1 << 13) - 1);
	data |= 0xccb;
	mmio_write_32((0xf6504000 + 0x058), data);

	mmio_write_32((0xf6504000 + 0x060), 0x1fff);
	mmio_write_32((0xf6504000 + 0x064), 0x1ffffff);
	mmio_write_32((0xf6504000 + 0x068), 0x7fffffff);
	mmio_write_32((0xf6504000 + 0x05c), 0x1);

	data = mmio_read_32((0xf6504000 + 0x054));
	data &= ~(0xf << 12);
	data |= 1 << 12;
	mmio_write_32((0xf6504000 + 0x054), data);
	dsb();


	data = mmio_read_32((0xf7032000 + 0x000));
	data &= ~(1 << 0);
	mmio_write_32((0xf7032000 + 0x000), data);

	mmio_write_32((0xf7032000 + 0x004), 0x5110207d);
	mmio_write_32((0xf7032000 + 0x134), 0x10000005);
	data = mmio_read_32((0xf7032000 + 0x134));


	data = mmio_read_32((0xf7032000 + 0x000));
	data |= (1 << 0);
	mmio_write_32((0xf7032000 + 0x000), data);

	mmio_write_32((0xf7032000 + 0x368), 0x100da);
	data = mmio_read_32((0xf7032000 + 0x378));
	data &= ~((1 << 7) - 1);
	data |= 0x6b;
	mmio_write_32((0xf7032000 + 0x378), data);
	dsb();
	do {
		data = mmio_read_32((0xf7032000 + 0x378));
		tmp = data & 0x7f;
		data = (data & (0x7f << 8)) >> 8;
		if (data != tmp)
			continue;
		data = mmio_read_32((0xf7032000 + 0x37c));
	} while (!(data & 1));

	data = mmio_read_32((0xf7032000 + 0x104));
	data &= ~((3 << 0) |
			(3 << 8));
	cpuext_cfg = 1;
	ddr_cfg = 1;
	data |= cpuext_cfg | (ddr_cfg << 8);
	mmio_write_32((0xf7032000 + 0x104), data);
	dsb();

	do {
		data = mmio_read_32((0xf7032000 + 0x104));
		tmp = (data & (3 << 16)) >> 16;
		if (cpuext_cfg != tmp)
			continue;
		tmp = (data & (3 << 24)) >> 24;
		if (ddr_cfg != tmp)
			continue;
		data = mmio_read_32((0xf7032000 + 0x000));
		data &= 1 << 28;
	} while (!data);

	data = mmio_read_32((0xf7032000 + 0x100));
	data &= ~(1 << 0);
	mmio_write_32((0xf7032000 + 0x100), data);
	dsb();
	do {
		data = mmio_read_32((0xf7032000 + 0x100));
		data &= (1 << 1);
	} while (data != (1 << 1));
	dsb();
	mdelay(1000);

	data = mmio_read_32((0xf6504000 + 0x054));
	data &= ~(1 << 28);
	mmio_write_32((0xf6504000 + 0x054), data);
	dsb();

	data = mmio_read_32((0xf7032000 + 0x110));
	data &= ~((1 << 4) |
			(3 << 12));
	mmio_write_32((0xf7032000 + 0x110), data);
	dsb();
}

static int cat_533mhz_800mhz(void)
{
	unsigned int data, i;
	unsigned int bdl[5];


	data = mmio_read_32((0xf712c000 + 0x1c8));
	data &= 0xfffff0f0;
	data |= 0x100f01;
	mmio_write_32((0xf712c000 + 0x1c8), data);

	for (i = 0; i < 0x20; i++) {
		mmio_write_32((0xf712c000 + 0x1d4), 0xc0000);
		data = (i << 0x10) + i;
		mmio_write_32((0xf712c000 + 0x140), data);
		mmio_write_32((0xf712c000 + 0x144), data);
		mmio_write_32((0xf712c000 + 0x148), data);
		mmio_write_32((0xf712c000 + 0x14c), data);
		mmio_write_32((0xf712c000 + 0x150), data);


		data = mmio_read_32((0xf712c000 + 0x070));
		data |= 0x80000;
		mmio_write_32((0xf712c000 + 0x070), data);
		data = mmio_read_32((0xf712c000 + 0x070));
		data &= 0xfff7ffff;
		mmio_write_32((0xf712c000 + 0x070), data);


		mmio_write_32((0xf712c000 + 0x004), 0x8000);
		dsb();
		mdelay(1);
		mmio_write_32((0xf712c000 + 0x004), 0x0);
		dsb();
		mdelay(1);
		mmio_write_32((0xf712c000 + 0x004), 0x801);
		dsb();
		mdelay(1);
		do {
			data = mmio_read_32((0xf712c000 + 0x004));
		} while (data & 1);

		data = mmio_read_32((0xf712c000 + 0x008));
		if (!(data & 0x400)) {
			mdelay(10);
			return 0;
		}
		tf_printf("WARN:  " "lpddr3 cat fail\n");
		data = mmio_read_32((0xf712c000 + 0x1d4));
		if ((data & 0x1f00) && ((data & 0x1f) == 0)) {
			bdl[0] = mmio_read_32((0xf712c000 + 0x140));
			bdl[1] = mmio_read_32((0xf712c000 + 0x144));
			bdl[2] = mmio_read_32((0xf712c000 + 0x148));
			bdl[3] = mmio_read_32((0xf712c000 + 0x14c));
			bdl[4] = mmio_read_32((0xf712c000 + 0x150));
			if ((!(bdl[0] & 0x1f001f)) || (!(bdl[1] & 0x1f001f)) ||
					(!(bdl[2] & 0x1f001f)) || (!(bdl[3] & 0x1f001f)) ||
					(!(bdl[4] & 0x1f001f))) {
				tf_printf("WARN:  " "lpddr3 cat deskew error\n");
				if (i == 0x1f) {
					tf_printf("WARN:  " "addrnbdl is max\n");
					return -22;
				}
				mmio_write_32((0xf712c000 + 0x008), 0x400);
			} else {
				tf_printf("WARN:  " "lpddr3 cat other error1\n");
				return -22;
			}
		} else {
			tf_printf("WARN:  " "lpddr3 cat other error2\n");
			return -22;
		}
	}
	return -22;
}

static void ddrx_rdet(void)
{
	unsigned int data, rdet, bdl[4];

	data = mmio_read_32((0xf712c000 + 0x0d0));
	data &= 0xf800ffff;
	data |= 0x8a0000;
	mmio_write_32((0xf712c000 + 0x0d0), data);
	dsb();
	mdelay(1);

	data = mmio_read_32((0xf712c000 + 0x0dc));
	data &= 0xfffffff0;
	data |= 0x4;
	mmio_write_32((0xf712c000 + 0x0dc), data);
	dsb();
	mdelay(1);


	data = mmio_read_32((0xf712c000 + 0x070));
	data |= 0x80000;
	mmio_write_32((0xf712c000 + 0x070), data);
	dsb();
	mdelay(10);
	data = mmio_read_32((0xf712c000 + 0x070));
	data &= 0xfff7ffff;
	mmio_write_32((0xf712c000 + 0x070), data);
	dsb();
	mdelay(10);

	mmio_write_32((0xf712c000 + 0x004), 0x8000);
	dsb();
	mdelay(10);
	mmio_write_32((0xf712c000 + 0x004), 0);
	dsb();
	mdelay(10);

	data = mmio_read_32((0xf712c000 + 0x0d0));
	data &= ~0xf0000000;
	data |= 0x80000000;
	mmio_write_32((0xf712c000 + 0x0d0), data);

	mmio_write_32((0xf712c000 + 0x004), 0x101);
	dsb();
	do {
		data = mmio_read_32((0xf712c000 + 0x004));
	} while (!(data & 1));
	data = mmio_read_32((0xf712c000 + 0x008));
	if (data & 0x100)
		tf_printf("WARN:    " "rdet lbs fail\n");

	mdelay(10);
	bdl[0] = mmio_read_32((0xf712c000 + 0x22c)) & 0x7f;
	bdl[1] = mmio_read_32((0xf712c000 + 0x2ac)) & 0x7f;
	bdl[2] = mmio_read_32((0xf712c000 + 0x32c)) & 0x7f;
	bdl[3] = mmio_read_32((0xf712c000 + 0x3ac)) & 0x7f;
	do {
		data = mmio_read_32((0xf712c000 + 0x22c));
		data &= ~0x7f;
		data |= bdl[0];
		mmio_write_32((0xf712c000 + 0x22c), data);
		data = mmio_read_32((0xf712c000 + 0x2ac));
		data &= ~0x7f;
		data |= bdl[1];
		mmio_write_32((0xf712c000 + 0x2ac), data);
		data = mmio_read_32((0xf712c000 + 0x32c));
		data &= ~0x7f;
		data |= bdl[2];
		mmio_write_32((0xf712c000 + 0x32c), data);
		data = mmio_read_32((0xf712c000 + 0x3ac));
		data &= ~0x7f;
		data |= bdl[3];
		mmio_write_32((0xf712c000 + 0x3ac), data);


		data = mmio_read_32((0xf712c000 + 0x070));
		data |= 0x80000;
		mmio_write_32((0xf712c000 + 0x070), data);
		dsb();
		data = mmio_read_32((0xf712c000 + 0x070));
		data &= 0xfff7ffff;
		mmio_write_32((0xf712c000 + 0x070), data);

		mmio_write_32((0xf712c000 + 0x004), 0x8000);
		dsb();
		mdelay(1);
		mmio_write_32((0xf712c000 + 0x004), 0);

		data = mmio_read_32((0xf712c000 + 0x0d0));
		data &= ~0xf0000000;
		data |= 0x40000000;
		mmio_write_32((0xf712c000 + 0x0d0), data);
		dsb();
		mmio_write_32((0xf712c000 + 0x004), 0x101);
		dsb();
		do {
			data = mmio_read_32((0xf712c000 + 0x004));
		} while (data & 1);

		data = mmio_read_32((0xf712c000 + 0x008));
		rdet = data & 0x100;
		if (rdet) {
			tf_printf("INFO:    " "rdet ds fail\n");
			mmio_write_32((0xf712c000 + 0x008), 0x100);
		}
		dsb();
		mdelay(10);
		bdl[0]++;
		bdl[1]++;
		bdl[2]++;
		bdl[3]++;
	} while (rdet);

	data = mmio_read_32((0xf712c000 + 0x0d0));
	data &= ~0xf0000000;
	data |= 0x30000000;
	mmio_write_32((0xf712c000 + 0x0d0), data);

	mmio_write_32((0xf712c000 + 0x004), 0x101);
	dsb();
	do {
		data = mmio_read_32((0xf712c000 + 0x004));
	} while (data & 1);
	data = mmio_read_32((0xf712c000 + 0x008));
	if (data & 0x100)
		tf_printf("INFO:    " "rdet rbs av fail\n");
	dsb();
	mdelay(10);
}

static void ddrx_wdet(void)
{
	unsigned int data, wdet, zero_bdl, dq[4];
	int i;

	data = mmio_read_32((0xf712c000 + 0x0d0));
	data &= ~0xf;
	data |= 0xa;
	mmio_write_32((0xf712c000 + 0x0d0), data);

	data = mmio_read_32((0xf712c000 + 0x070));
	data |= 0x80000;
	mmio_write_32((0xf712c000 + 0x070), data);
	data = mmio_read_32((0xf712c000 + 0x070));
	data &= ~0x80000;
	mmio_write_32((0xf712c000 + 0x070), data);

	mmio_write_32((0xf712c000 + 0x004), 0x8000);
	mmio_write_32((0xf712c000 + 0x004), 0);
	data = mmio_read_32((0xf712c000 + 0x0d0));
	data &= ~0xf000;
	data |= 0x8000;
	mmio_write_32((0xf712c000 + 0x0d0), data);
	mmio_write_32((0xf712c000 + 0x004), 0x201);
	dsb();
	do {
		data = mmio_read_32((0xf712c000 + 0x004));
	} while (data & 1);
	data = mmio_read_32((0xf712c000 + 0x008));
	if (data & 0x200)
		tf_printf("INFO:    " "wdet lbs fail\n");
	mdelay(10);

	dq[0] = mmio_read_32((0xf712c000 + 0x234)) & 0x1f00;
	dq[1] = mmio_read_32((0xf712c000 + 0x2b4)) & 0x1f00;
	dq[2] = mmio_read_32((0xf712c000 + 0x334)) & 0x1f00;
	dq[3] = mmio_read_32((0xf712c000 + 0x3b4)) & 0x1f00;

	do {
		mmio_write_32((0xf712c000 + 0x234), dq[0]);
		mmio_write_32((0xf712c000 + 0x2b4), dq[1]);
		mmio_write_32((0xf712c000 + 0x334), dq[2]);
		mmio_write_32((0xf712c000 + 0x3b4), dq[3]);

		data = mmio_read_32((0xf712c000 + 0x070));
		data |= 0x80000;
		mmio_write_32((0xf712c000 + 0x070), data);
		data = mmio_read_32((0xf712c000 + 0x070));
		data &= ~0x80000;
		mmio_write_32((0xf712c000 + 0x070), data);
		mmio_write_32((0xf712c000 + 0x004), 0x8000);
		mmio_write_32((0xf712c000 + 0x004), 0);

		data = mmio_read_32((0xf712c000 + 0x0d0));
		data &= ~0xf000;
		data |= 0x4000;
		mmio_write_32((0xf712c000 + 0x0d0), data);
		mmio_write_32((0xf712c000 + 0x004), 0x201);
		do {
			data = mmio_read_32((0xf712c000 + 0x004));
		} while (data & 1);

		data = mmio_read_32((0xf712c000 + 0x008));
		wdet = data & 0x200;
		if (wdet) {
			tf_printf("INFO:    " "wdet ds fail\n");
			mmio_write_32((0xf712c000 + 0x008), 0x200);
		}
		mdelay(10);

		for (i = 0; i < 4; i++) {
			data = mmio_read_32((0xf712c000 + 0x210 + i * 0x80));
			if ((!(data & 0x1f)) || (!(data & 0x1f00)) ||
					(!(data & 0x1f0000)) || (!(data & 0x1f000000)))
				zero_bdl = 1;
			data = mmio_read_32((0xf712c000 + 0x214 + i * 0x80));
			if ((!(data & 0x1f)) || (!(data & 0x1f00)) ||
					(!(data & 0x1f0000)) || (!(data & 0x1f000000)))
				zero_bdl = 1;
			data = mmio_read_32((0xf712c000 + 0x218 + i * 0x80));
			if (!(data & 0x1f))
				zero_bdl = 1;
			if (zero_bdl) {
				if (i == 0)
					dq[0] = dq[0] - 0x100;
				if (i == 1)
					dq[1] = dq[1] - 0x100;
				if (i == 2)
					dq[2] = dq[2] - 0x100;
				if (i == 3)
					dq[3] = dq[3] - 0x100;
			}
		}
	} while (wdet);

	data = mmio_read_32((0xf712c000 + 0x0d0));
	data &= ~0xf000;
	data |= 0x3000;
	mmio_write_32((0xf712c000 + 0x0d0), data);
	mmio_write_32((0xf712c000 + 0x004), 0x201);
	do {
		data = mmio_read_32((0xf712c000 + 0x004));
	} while (data & 1);
	data = mmio_read_32((0xf712c000 + 0x008));
	if (data & 0x200)
		tf_printf("INFO:    " "wdet rbs av fail\n");
	mdelay(10);
}

#ifndef DDR800
static void set_ddrc_533mhz(void)
{
	unsigned int data;

	mmio_write_32((0xf7032000 + 0x580), 0x3);
	mmio_write_32((0xf7032000 + 0x5a8), 0x11111);
	data = mmio_read_32((0xf7032000 + 0x104));
	data |= 0x100;
	mmio_write_32((0xf7032000 + 0x104), data);

	mmio_write_32((0xf7030000 + 0x050), 0x30);
	mmio_write_32((0xf7030000 + 0x240), 0x5ffff);
	mmio_write_32((0xf7030000 + 0x344), 0xf5ff);
	mmio_write_32((0xf712c000 + 0x00c), 0x400);
	dsb();
	udelay(100);
	mmio_write_32((0xf712c000 + 0x018), 0x7);
	mmio_write_32((0xf712c000 + 0x090), 0x6400000);
	mmio_write_32((0xf712c000 + 0x258), 0x640);
	mmio_write_32((0xf712c000 + 0x2d8), 0x640);
	mmio_write_32((0xf712c000 + 0x358), 0x640);
	mmio_write_32((0xf712c000 + 0x3d8), 0x640);
	mmio_write_32((0xf712c000 + 0x018), 0x0);
	mmio_write_32((0xf712c000 + 0x0b0), 0xf00000f);
	mmio_write_32((0xf712c000 + 0x0b4), 0xf);
	mmio_write_32((0xf712c000 + 0x088), 0x3fff801);
	mmio_write_32((0xf712c000 + 0x070), 0x8940000);

	data = mmio_read_32((0xf712c000 + 0x078));
	data |= 4;
	mmio_write_32((0xf712c000 + 0x078), data);
	mmio_write_32((0xf712c000 + 0x01c), 0x8000080);
	data = mmio_read_32((0xf712c000 + 0x020));
	data &= 0xfffffffe;
	mmio_write_32((0xf712c000 + 0x020), data);
	mmio_write_32((0xf712c000 + 0x1d4), 0xc0000);
	mmio_write_32((0xf712c000 + 0x010), 0x500000f);
	mmio_write_32((0xf712c000 + 0x014), 0x10);
	data = mmio_read_32((0xf712c000 + 0x1e4));
	data &= 0xffffff00;
	mmio_write_32((0xf712c000 + 0x1e4), data);
	mmio_write_32((0xf712c000 + 0x030), 0x9dd87855);
	mmio_write_32((0xf712c000 + 0x034), 0xa7138bb);
	mmio_write_32((0xf712c000 + 0x038), 0x20091477);
	mmio_write_32((0xf712c000 + 0x03c), 0x84534e16);
	mmio_write_32((0xf712c000 + 0x040), 0x3008817);
	mmio_write_32((0xf712c000 + 0x064), 0x106c3);
	mmio_write_32((0xf712c000 + 0x068), 0xff0a0000);
	data = mmio_read_32((0xf712c000 + 0x070));
	data &= 0xffff0000;
	data |= 0x305;
	mmio_write_32((0xf712c000 + 0x070), data);
	data = mmio_read_32((0xf712c000 + 0x048));
	data |= 0x40000000;
	mmio_write_32((0xf712c000 + 0x048), data);
	data = mmio_read_32((0xf712c000 + 0x020));
	data &= ~0x10;
	mmio_write_32((0xf712c000 + 0x020), data);
	data = mmio_read_32((0xf712c000 + 0x080));
	data &= ~0x2000;
	mmio_write_32((0xf712c000 + 0x080), data);
	mmio_write_32((0xf712c000 + 0x270), 0x3);
	mmio_write_32((0xf712c000 + 0x2f0), 0x3);
	mmio_write_32((0xf712c000 + 0x370), 0x3);
	mmio_write_32((0xf712c000 + 0x3f0), 0x3);
	mmio_write_32((0xf712c000 + 0x048), 0xd0420900);

	mmio_write_32((0xf7128000 + 0x040), 0x0);
	mmio_write_32((0xf712c000 + 0x004), 0x140f);
	do {
		data = mmio_read_32((0xf712c000 + 0x004));
	} while (data & 1);
	data = mmio_read_32((0xf712c000 + 0x008));
	if (data & 0x7fe) {
		tf_printf("NOTICE:  " "failed to init lpddr3 rank0 dram phy\n");
		return;
	}
	tf_printf("NOTICE:  " "succeed to init lpddr3 rank0 dram phy\n");
}
#endif

#ifdef DDR800
static void set_ddrc_800mhz(void)
{
	unsigned int data;

	mmio_write_32((0xf7032000 + 0x580), 0x2);
	mmio_write_32((0xf7032000 + 0x5a8), 0x1003);
	data = mmio_read_32((0xf7032000 + 0x104));
	data &= 0xfffffcff;
	mmio_write_32((0xf7032000 + 0x104), data);
	dsb();
	mdelay(10);

	mmio_write_32((0xf7030000 + 0x050), 0x30);
	mmio_write_32((0xf7030000 + 0x240), 0x5ffff);
	mmio_write_32((0xf7030000 + 0x344), 0xf5ff);
	dsb();
	mmio_write_32((0xf712c000 + 0x00c), 0x400);
	mmio_write_32((0xf712c000 + 0x018), 0x7);
	dsb();
	mmio_write_32((0xf712c000 + 0x090), 0x5400000);
	mmio_write_32((0xf712c000 + 0x258), 0x540);
	mmio_write_32((0xf712c000 + 0x2d8), 0x540);
	mmio_write_32((0xf712c000 + 0x358), 0x540);
	mmio_write_32((0xf712c000 + 0x3d8), 0x540);
	mmio_write_32((0xf712c000 + 0x018), 0x0);
	mmio_write_32((0xf712c000 + 0x0b0), 0xf00000f);
	mmio_write_32((0xf712c000 + 0x0b4), 0xf);
	mmio_write_32((0xf712c000 + 0x088), 0x3fff801);
	mmio_write_32((0xf712c000 + 0x070), 0x8940000);

	data = mmio_read_32((0xf712c000 + 0x078));
	data |= 4;
	mmio_write_32((0xf712c000 + 0x078), data);
	mmio_write_32((0xf712c000 + 0x01c), 0x8000080);
	data = mmio_read_32((0xf712c000 + 0x020));
	data &= 0xfffffffe;
	mmio_write_32((0xf712c000 + 0x020), data);
	mmio_write_32((0xf712c000 + 0x1d4), 0xc0000);
	mmio_write_32((0xf712c000 + 0x010), 0x500000f);
	mmio_write_32((0xf712c000 + 0x014), 0x10);
	data = mmio_read_32((0xf712c000 + 0x1e4));
	data &= 0xffffff00;
	mmio_write_32((0xf712c000 + 0x1e4), data);
	mmio_write_32((0xf712c000 + 0x030), 0xe663ab77);
	mmio_write_32((0xf712c000 + 0x034), 0xea952db);
	mmio_write_32((0xf712c000 + 0x038), 0x200d1cb1);
	mmio_write_32((0xf712c000 + 0x03c), 0xc67d0721);
	mmio_write_32((0xf712c000 + 0x040), 0x3008aa1);
	mmio_write_32((0xf712c000 + 0x064), 0x11a43);
	mmio_write_32((0xf712c000 + 0x068), 0xff0a0000);
	data = mmio_read_32((0xf712c000 + 0x070));
	data &= 0xffff0000;
	data |= 0x507;
	mmio_write_32((0xf712c000 + 0x070), data);
	data = mmio_read_32((0xf712c000 + 0x048));
	data |= 0x40000000;
	mmio_write_32((0xf712c000 + 0x048), data);
	data = mmio_read_32((0xf712c000 + 0x020));
	data &= 0xffffffef;
	mmio_write_32((0xf712c000 + 0x020), data);
	data = mmio_read_32((0xf712c000 + 0x080));
	data &= 0xffffdfff;
	mmio_write_32((0xf712c000 + 0x080), data);
	mmio_write_32((0xf712c000 + 0x270), 0x3);
	mmio_write_32((0xf712c000 + 0x2f0), 0x3);
	mmio_write_32((0xf712c000 + 0x370), 0x3);
	mmio_write_32((0xf712c000 + 0x3f0), 0x3);
	mmio_write_32((0xf712c000 + 0x048), 0xd0420900);
	dsb();

	mmio_write_32((0xf7128000 + 0x040), 0x2001);
	mmio_write_32((0xf712c000 + 0x004), 0x140f);
	dsb();
	do {
		data = mmio_read_32((0xf712c000 + 0x004));
	} while (data & 1);
	data = mmio_read_32((0xf712c000 + 0x008));
	if (data & 0x7fe) {
		WARN("failed to init lpddr3 rank0 dram phy\n");
		return;
	}
}
#endif

static void ddrc_common_init(void)
{
	unsigned int data;

	mmio_write_32((0xf7120000 + 0x020), 0x1);
	mmio_write_32((0xf7120000 + 0x100), 0x1700);
	mmio_write_32((0xf7120000 + 0x104), 0x71040004);
	dsb();
	mmio_write_32((0xf7121400 + 0x104), 0xf);
	dsb();
	mmio_write_32((0xf7121800 + 0x104), 0xf);
	mmio_write_32((0xf7121800 + 0x104), 0xf);
	dsb();
	mmio_write_32((0xf7121c00 + 0x104), 0xf);
	dsb();
	mmio_write_32((0xf7122000 + 0x104), 0xf);
	dsb();
	mmio_write_32((0xf7128000 + 0x02c), 0x6);
	mmio_write_32((0xf7128000 + 0x020), 0x1);
	mmio_write_32((0xf7128000 + 0x028), 0x310201);
	dsb();
	mmio_write_32((0xf712c000 + 0x1e4), 0xfe007600);
	dsb();
	mmio_write_32((0xf7128000 + 0x01c), 0xaf001);


	data = mmio_read_32((0xf7128000 + 0x280));
	data |= 1 << 7;
	mmio_write_32((0xf7128000 + 0x280), data);
	mmio_write_32((0xf7128000 + 0x244), 0x3);

#ifdef DDR800
	mmio_write_32((0xf7128000 + 0x240), 167 * 400000 / 1024);
#else
	mmio_write_32((0xf7128000 + 0x240), 167 * 533000 / 1024);
#endif

	dsb();
	data = mmio_read_32((0xf712c000 + 0x080));
	data &= 0xffff;
	data |= 0x4002000;
	mmio_write_32((0xf712c000 + 0x080), data);
	dsb();
	mmio_write_32((0xf7128000 + 0x000), 0x0);
	dsb();
	do {
		data = mmio_read_32((0xf7128000 + 0x294));
	} while (data & 1);
	mmio_write_32((0xf7128000 + 0x000), 0x2);
}


static int dienum_det_and_rowcol_cfg(void)
{
	unsigned int data;

	mmio_write_32((0xf7128000 + 0x210), 0x87);
	mmio_write_32((0xf7128000 + 0x218), 0x10000);
	mmio_write_32((0xf7128000 + 0x00c), 0x1);
	do {
		data = mmio_read_32((0xf7128000 + 0x00c));
	} while (data & 1);
	data = mmio_read_32((0xf7128000 + 0x4a8)) & 0xfc;
	switch (data) {
		case 0x18:
			mmio_write_32((0xf7128000 + 0x060), 0x132);
			mmio_write_32((0xf7128000 + 0x064), 0x132);
			mmio_write_32((0xf7120000 + 0x100), 0x1600);
			mmio_write_32((0xf7120000 + 0x104), 0x71040004);
			break;
		case 0x1c:
			mmio_write_32((0xf7128000 + 0x060), 0x142);
			mmio_write_32((0xf7128000 + 0x064), 0x142);
			mmio_write_32((0xf7120000 + 0x100), 0x1700);
			mmio_write_32((0xf7120000 + 0x104), 0x71040004);
			break;
		case 0x58:
			mmio_write_32((0xf7128000 + 0x060), 0x133);
			mmio_write_32((0xf7128000 + 0x064), 0x133);
			mmio_write_32((0xf7120000 + 0x100), 0x1700);
			mmio_write_32((0xf7120000 + 0x104), 0x71040004);
			break;
		default:
			break;
	}
	if (!data)
		return -22;
	return 0;
}

static int detect_ddr_chip_info(void)
{
	unsigned int data, mr5, mr6, mr7;

	mmio_write_32((0xf7128000 + 0x210), 0x57);
	mmio_write_32((0xf7128000 + 0x218), 0x10000);
	mmio_write_32((0xf7128000 + 0x00c), 0x1);

	do {
		data = mmio_read_32((0xf7128000 + 0x00c));
	} while (data & 1);

	data = mmio_read_32((0xf7128000 + 0x4a8));
	mr5 = data & 0xff;
	switch (mr5) {
		case 1:
			tf_printf("INFO:    " "Samsung DDR\n");
			break;
		case 6:
			tf_printf("INFO:    " "Hynix DDR\n");
			break;
		case 3:
			tf_printf("INFO:    " "Elpida DDR\n");
			break;
		default:
			tf_printf("INFO:    " "DDR from other vendors\n");
			break;
	}

	mmio_write_32((0xf7128000 + 0x210), 0x67);
	mmio_write_32((0xf7128000 + 0x218), 0x10000);
	mmio_write_32((0xf7128000 + 0x00c), 0x1);
	do {
		data = mmio_read_32((0xf7128000 + 0x00c));
	} while (data & 1);
	data = mmio_read_32((0xf7128000 + 0x4a8));
	mr6 = data & 0xff;
	mmio_write_32((0xf7128000 + 0x210), 0x77);
	mmio_write_32((0xf7128000 + 0x218), 0x10000);
	mmio_write_32((0xf7128000 + 0x00c), 0x1);
	do {
		data = mmio_read_32((0xf7128000 + 0x00c));
	} while (data & 1);
	data = mmio_read_32((0xf7128000 + 0x4a8));
	mr7 = data & 0xff;
	data = mr5 + (mr6 << 8) + (mr7 << 16);
	return data;
}

static int lpddr3_freq_init(void)
{
	unsigned int data;

#ifdef DDR800
	set_ddrc_800mhz();
	tf_printf("INFO:    " "%s, set ddrc 800mhz\n", __func__);
#else
	set_ddrc_533mhz();
	tf_printf("INFO:    " "%s, set ddrc 533mhz\n", __func__);
#endif

	data = cat_533mhz_800mhz();
	if (data)
		tf_printf("NOTICE:  " "fail to set eye diagram\n");

	mmio_write_32((0xf712c000 + 0x004), 0xf1);
	mmio_write_32((0xf7128000 + 0x050), 0x100123);
	mmio_write_32((0xf7128000 + 0x060), 0x133);
	mmio_write_32((0xf7128000 + 0x064), 0x133);
	mmio_write_32((0xf7128000 + 0x200), 0xa1000);
#ifdef DDR800
	mmio_write_32((0xf7128000 + 0x100), 0x755a9d12);
	mmio_write_32((0xf7128000 + 0x104), 0x1753b055);
	mmio_write_32((0xf7128000 + 0x108), 0x7401505f);
	mmio_write_32((0xf7128000 + 0x10c), 0x578ca244);
	mmio_write_32((0xf7128000 + 0x110), 0x10700000);
	mmio_write_32((0xf7128000 + 0x114), 0x13141306);
#else
	mmio_write_32((0xf7128000 + 0x100), 0xb77b6718);
	mmio_write_32((0xf7128000 + 0x104), 0x1e82a071);
	mmio_write_32((0xf7128000 + 0x108), 0x9501c07e);
	mmio_write_32((0xf7128000 + 0x10c), 0xaf50c255);
	mmio_write_32((0xf7128000 + 0x110), 0x10b00000);
	mmio_write_32((0xf7128000 + 0x114), 0x13181908);
#endif
	mmio_write_32((0xf7128000 + 0x118), 0x44);
	do {
		data = mmio_read_32((0xf712c000 + 0x004));
	} while (data & 1);

	data = mmio_read_32((0xf712c000 + 0x008));
	if (data & 0x7fe) {
		tf_printf("NOTICE:  " "fail to init ddr3 rank0 in 533MHz\n");
		return -14;
	}
	tf_printf("INFO:    " "init ddr3 rank0 in 533MHz\n");
	ddrx_rdet();
	ddrx_wdet();

	data = mmio_read_32((0xf712c000 + 0x048));
	data |= 1;
	mmio_write_32((0xf712c000 + 0x048), data);
	mmio_write_32((0xf712c000 + 0x004), 0x21);
	do {
		data = mmio_read_32((0xf712c000 + 0x004));
	} while (data & 1);

	data = mmio_read_32((0xf712c000 + 0x008));
	if (data & 0x7fe)
		tf_printf("NOTICE:  " "ddr3 rank1 init faile in 533MHz\n");
	else
		tf_printf("INFO:    " "ddr3 rank1 init pass in 533MHz\n");

	data = mmio_read_32((0xf712c000 + 0x048));
	data &= ~0xf;
	mmio_write_32((0xf712c000 + 0x048), data);
	return 0;
}

static void init_ddr(void)
{
	unsigned int data;
	int ret;


	data = mmio_read_32((0xf7032000 + 0x030));
	data |= 1;
	mmio_write_32((0xf7032000 + 0x030), data);
	dsb();
	mdelay(10);
	data = mmio_read_32((0xf7032000 + 0x010));
	data |= 1;
	mmio_write_32((0xf7032000 + 0x010), data);

	dsb();
	mdelay(10);

	do {
		data = mmio_read_32((0xf7032000 + 0x030));
		data &= 3 << 28;
	} while (data != (3 << 28));
	do {
		data = mmio_read_32((0xf7032000 + 0x010));
		data &= 3 << 28;
	} while (data != (3 << 28));

	ret = lpddr3_freq_init();
	if (ret)
		return;

	ddrc_common_init();
	dienum_det_and_rowcol_cfg();
	detect_ddr_chip_info();


#ifdef DDR800
	data = mmio_read_32(0xf7032000 + 0x010);
	data &= ~0x1;
	mmio_write_32(0xf7032000 + 0x010, data);
	data = mmio_read_32(0xf7032000 + 0x010);
#else
	data = mmio_read_32((0xf7032000 + 0x030));
	data &= ~0x1;
	mmio_write_32((0xf7032000 + 0x030), data);
	data = mmio_read_32((0xf7032000 + 0x030));
#endif
}

static void init_ddrc_qos(void)
{
	unsigned int port, data;

	mmio_write_32((0xf7124000 + 0x088), 1);


	port = 0;
	mmio_write_32((0xf7120000 + 0x200 + port * 0x10), 0x1210);
	mmio_write_32((0xf7120000 + 0x204 + port * 0x10), 0x11111111);
	mmio_write_32((0xf7120000 + 0x208 + port * 0x10), 0x11111111);
	mmio_write_32((0xf7120000 + 0x400 + 0 * 0x10), 0x001d0007);


	for (port = 3; port <= 4; port++) {
		mmio_write_32((0xf7120000 + 0x200 + port * 0x10), 0x1210);
		mmio_write_32((0xf7120000 + 0x204 + port * 0x10), 0x77777777);
		mmio_write_32((0xf7120000 + 0x208 + port * 0x10), 0x77777777);
	}


	port = 1;
	mmio_write_32((0xf7120000 + 0x200 + port * 0x10), 0x30000);
	mmio_write_32((0xf7120000 + 0x204 + port * 0x10), 0x1234567);
	mmio_write_32((0xf7120000 + 0x208 + port * 0x10), 0x1234567);


	mmio_write_32((0xf7124000 + 0x1f0), 0);
	mmio_write_32((0xf7124000 + 0x0bc), 0x3020100);
	mmio_write_32((0xf7124000 + 0x0d0), 0x3020100);
	mmio_write_32((0xf7124000 + 0x1f4), 0x01000100);
	mmio_write_32((0xf7124000 + 0x08c + 0 * 4), 0xd0670402);
	mmio_write_32((0xf7124000 + 0x068 + 0 * 4), 0x31);
	mmio_write_32((0xf7124000 + 0x000), 0x7);

	data = mmio_read_32((0xf7124000 + 0x09c));
	data &= ~0xff0000;
	data |= 0x400000;
	mmio_write_32((0xf7124000 + 0x09c), data);
	data = mmio_read_32((0xf7124000 + 0x0ac));
	data &= ~0xff0000;
	data |= 0x400000;
	mmio_write_32((0xf7124000 + 0x0ac), data);
	port = 2;
	mmio_write_32((0xf7120000 + 0x200 + port * 0x10), 0x30000);
	mmio_write_32((0xf7120000 + 0x204 + port * 0x10), 0x1234567);
	mmio_write_32((0xf7120000 + 0x208 + port * 0x10), 0x1234567);


	mmio_write_32((0xf7124000 + 0x09c), 0xff7fff);
	mmio_write_32((0xf7124000 + 0x0a0), 0xff);
	mmio_write_32((0xf7124000 + 0x0ac), 0xff7fff);
	mmio_write_32((0xf7124000 + 0x0b0), 0xff);
	mmio_write_32((0xf7124000 + 0x0bc), 0x3020100);
	mmio_write_32((0xf7124000 + 0x0d0), 0x3020100);
}

static void init_mmc_pll(void)
{
	unsigned int data;

	data = hi6553_read_8(0x084);
	data |= 0x7;
	hi6553_write_8(0x084, data);

	/* select SYSPLL as the source of MMC0 */
	/* select SYSPLL as the source of MUX1 (SC_CLK_SEL0) */
	mmio_write_32(PERI_SC_CLK_SEL0, 1 << 5 | 1 << 21);
	do {
		data = mmio_read_32(PERI_SC_CLK_SEL0);
	} while (!(data & (1 << 5)));
	/* select MUX1 as the source of MUX2 (SC_CLK_SEL0) */
	mmio_write_32(PERI_SC_CLK_SEL0, 1 << 29);
	do {
		data = mmio_read_32(PERI_SC_CLK_SEL0);
	} while (data & (1 << 13));

	mmio_write_32((0xf7030000 + 0x200), (1 << 0));
	dsb();
	do {
		data = mmio_read_32((0xf7030000 + 0x208));
	} while (!(data & (1 << 0)));

	data = mmio_read_32((0xf7030000 + 0x270));
	data |= 1 << 1;
	mmio_write_32((0xf7030000 + 0x270), data);
	dsb();

	do {
		mmio_write_32(PERI_SC_CLKCFG8BIT1, (1 << 7) | 0xb);
		data = mmio_read_32(PERI_SC_CLKCFG8BIT1);
	} while ((data & 0xb) != 0xb);
}

static void reset_mmc0_clk(void)
{
	unsigned int data;

	/* disable mmc0 bus clock */
	mmio_write_32(PERI_SC_PERIPH_CLKDIS0, PERI_CLK_MMC0);
	do {
		data = mmio_read_32(PERI_SC_PERIPH_CLKSTAT0);
	} while (data & PERI_CLK_MMC0);
	/* enable mmc0 bus clock */
	mmio_write_32(PERI_SC_PERIPH_CLKEN0, PERI_CLK_MMC0);
	do {
		data = mmio_read_32(PERI_SC_PERIPH_CLKSTAT0);
	} while (!(data & PERI_CLK_MMC0));
	/* reset mmc0 clock domain */
	mmio_write_32(PERI_SC_PERIPH_RSTEN0, PERI_CLK_MMC0);

	/* bypass mmc0 clock phase */
	data = mmio_read_32(PERI_SC_PERIPH_CTRL2);
	data |= 3;
	mmio_write_32(PERI_SC_PERIPH_CTRL2, data);

	/* disable low power */
	data = mmio_read_32(PERI_SC_PERIPH_CTRL13);
	data |= 1 << 3;
	mmio_write_32(PERI_SC_PERIPH_CTRL13, data);
	do {
		data = mmio_read_32(PERI_SC_PERIPH_RSTSTAT0);
	} while (!(data & (1 << 0)));

	/* unreset mmc0 clock domain */
	mmio_write_32(PERI_SC_PERIPH_RSTDIS0, 1 << 0);
	do {
		data = mmio_read_32(PERI_SC_PERIPH_RSTSTAT0);
	} while (data & (1 << 0));
}

static void init_media_clk(void)
{
	unsigned int data, value;

	data = mmio_read_32(PMCTRL_MEDPLLCTRL);
	data |= 1;
	mmio_write_32(PMCTRL_MEDPLLCTRL, data);

	for (;;) {
		data = mmio_read_32(PMCTRL_MEDPLLCTRL);
		value = 1 << 28;
		if ((data & value) == value)
			break;
	}

	data = mmio_read_32(PERI_SC_PERIPH_CLKEN12);
	data = 1 << 10;
	mmio_write_32(PERI_SC_PERIPH_CLKEN12, data);
}

void hi6220_pll_init(void)
{
	init_pll();
	init_freq();
	init_ddr();
	init_ddrc_qos();

	/*
	 * Test memory access. Do not use address 0x0 because the compiler
	 * may assume it is not a valid address and generate incorrect code
	 * (GCC 4.9.1 without -fno-delete-null-pointer-checks for instance).
	 */
	mmio_write_32(0x4, 0xa5a55a5a);
	INFO("ddr test value:0x%x\n", mmio_read_32(0x4));

	init_mmc_pll();
	reset_mmc0_clk();
	init_media_clk();

	dsb();
	isb();
}
