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

#ifndef __HISI_HKADC_H__
#define __HISI_HKADC_H__

/* hkadc channel */
#define HKADC_CHAN_UNUSE0			(0x00)
#define HKADC_CHAN_BOARDID1			(0x01)
#define HKADC_CHAN_BOARDID2			(0x02)
#define HKADC_CHAN_BOARDID0			(0x04)
#define HKADC_CHAN_VCOINMON			(0x06)
#define HKADC_CHAN_RTMP				(0x0A)
#define HKADC_CHAN_VBATMON			(0x0B)

/* hkadc registers */
#define HKADC_BASE				(0xF7013000)

#define HKADC_DSP_START				(HKADC_BASE + 0x0000)
#define HKADC_DSP_CFG				(HKADC_BASE + 0x0004)
#define HKADC_WR_NUM				(HKADC_BASE + 0x0008)
#define HKADC_DSP_WAIT_TIME			(HKADC_BASE + 0x000C)
#define HKADC_TIMEOUT_ERR_CLR			(HKADC_BASE + 0x0010)
#define HKADC_TIMEOUT_ERR			(HKADC_BASE + 0x0018)
#define HKADC_DSP_START_CLR			(HKADC_BASE + 0x001C)
#define HKADC_WR01_DATA				(HKADC_BASE + 0x0020)
#define HKADC_WR23_DATA				(HKADC_BASE + 0x0024)
#define HKADC_WR45_DATA				(HKADC_BASE + 0x0028)
#define HKADC_WR67_DATA				(HKADC_BASE + 0x002C)
#define HKADC_DELAY01				(HKADC_BASE + 0x0030)
#define HKADC_DELAY23				(HKADC_BASE + 0x0034)
#define HKADC_DELAY45				(HKADC_BASE + 0x0038)
#define HKADC_DELAY6				(HKADC_BASE + 0x003C)
#define HKADC_DSP_RD0_DATA			(HKADC_BASE + 0x0040)
#define HKADC_DSP_RD1_DATA			(HKADC_BASE + 0x0044)
#define HKADC_DSP_RD2_DATA			(HKADC_BASE + 0x0048)
#define HKADC_DSP_RD3_DATA			(HKADC_BASE + 0x004C)
#define HKADC_DSP_RD4_DATA			(HKADC_BASE + 0x0050)
#define HKADC_DSP_RD5_DATA			(HKADC_BASE + 0x0054)
#define HKADC_DSP_RD6_DATA			(HKADC_BASE + 0x0058)
#define HKADC_DSP_RD7_DATA			(HKADC_BASE + 0x005C)
#define HKADC_OP_INTERVAL			(HKADC_BASE + 0x0060)
#define HKADC_OP_INTERVAL_BYPASS		(HKADC_BASE + 0x0064)
#define HKADC_CHANNEL_EN			(HKADC_BASE + 0x0068)
#define HKADC_DBG_INFO				(HKADC_BASE + 0x00D0)
#define HKADC_FINSH_RAW_INT			(HKADC_BASE + 0x0100)
#define HKADC_FINSH_MSK_INT			(HKADC_BASE + 0x0104)
#define HKADC_FINSH_INT_CLR			(HKADC_BASE + 0x0108)
#define HKADC_FINSH_INT_MSK			(HKADC_BASE + 0x010C)

#define PMU_HKADC_CFG_ADDR			(0x00)
#define PMU_HKADC_START_ADDR			(0x01)
#define PMU_HKADC_STATUS_ADDR			(0x02)
#define PMU_HKADC_RDHIGH_ADDR			(0x03)
#define PMU_HKADC_RDLOW_ADDR			(0x04)

#define HKADC_WR01_VALUE 			\
	((0x0 << 31) |				\
	 (PMU_HKADC_START_ADDR << 24) |		\
	 (0x1 << 16) |				\
	 (0x0 << 15) | 				\
	 (PMU_HKADC_CFG_ADDR << 8) |		\
	 (0x3 << 5))

#define HKADC_WR23_VALUE			\
	((0x1 << 31) |				\
	 (PMU_HKADC_RDHIGH_ADDR << 24) | 	\
	 (0x1 << 15) |				\
	 (PMU_HKADC_RDLOW_ADDR << 8))

#define HKADC_WR45_VALUE			\
	((0x0 << 15) |				\
	 (PMU_HKADC_CFG_ADDR << 8) |		\
	 (0x80 << 0))

#define HKADC_WR_NUM_VALUE			(5)
#define HKADC_DELAY01_VALUE			((0x0700 << 16) | (0x0200 << 0))
#define HKADC_DELAY23_VALUE			((0x00c8 << 16) | (0x00c8 << 0))
#define HKADC_START_TIMEOUT			(2000)

#define HKADC_VALUE_HIGH			(0x0FF0)
#define HKADC_VALUE_LOW				(0x000F)
#define HKADC_VALID_VALUE			(0x0FFF)
#define HKADC_VREF_1V8				1800

#define HKADC_DATA_VOLT_GRADE0			(0)
#define HKADC_DATA_VOLT_GRADE1			(100)
#define HKADC_DATA_VOLT_GRADE2			(300)
#define HKADC_DATA_VOLT_GRADE3			(500)
#define HKADC_DATA_VOLT_GRADE4			(700)
#define HKADC_DATA_VOLT_GRADE5			(900)
#define HKADC_DATA_VOLT_GRADE6			(1100)
#define HKADC_DATA_VOLT_GRADE7			(1300)
#define HKADC_DATA_VOLT_GRADE8			(1500)
#define HKADC_DATA_VOLT_GRADE9			(1700)
#define HKADC_DATA_VOLT_GRADE10			(1800)

#define BOUNDARY_VALUE0				HKADC_DATA_VOLT_GRADE1
#define BOUNDARY_VALUE1				HKADC_DATA_VOLT_GRADE2
#define BOUNDARY_VALUE2				HKADC_DATA_VOLT_GRADE3
#define BOUNDARY_VALUE3				HKADC_DATA_VOLT_GRADE4
#define BOUNDARY_VALUE4				HKADC_DATA_VOLT_GRADE5
#define BOUNDARY_VALUE5				HKADC_DATA_VOLT_GRADE6
#define BOUNDARY_VALUE6				HKADC_DATA_VOLT_GRADE7
#define BOUNDARY_VALUE7				HKADC_DATA_VOLT_GRADE8
#define BOUNDARY_VALUE8				HKADC_DATA_VOLT_GRADE9
#define BOUNDARY_VALUE9				HKADC_DATA_VOLT_GRADE10

#define BOARDID_VALUE0				(0x0)
#define BOARDID_VALUE1				(0x1)
#define BOARDID_VALUE2				(0x2)
#define BOARDID_VALUE3				(0x3)
#define BOARDID_VALUE4				(0x4)
#define BOARDID_VALUE5				(0x5)
#define BOARDID_VALUE6				(0x6)
#define BOARDID_VALUE7				(0x7)
#define BOARDID_VALUE8				(0x8)
#define BOARDID_VALUE9				(0x9)
#define BOARDID_UNKNOW				(0xF)

extern void init_hkadc(void);
extern void init_boardid(void);

#endif /* __HISI_HKADC_H__ */
