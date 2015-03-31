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
#include <assert.h>
#include <bl_common.h>
#include <console.h>
#include <debug.h>
#include <hi6220.h>
#include <mmio.h>
#include <partitions.h>
#include <platform.h>
#include <platform_def.h>
#include <sp804_timer.h>
#include <string.h>

void init_hkadc(void)
{
	/* reset hkadc */
	mmio_write_32(PERI_SC_PERIPH_RSTEN3,  PERI_RST3_HKADC);
	udelay(2);
	mmio_write_32(PERI_SC_PERIPH_RSTDIS3, PERI_RST3_HKADC);
	udelay(2);

	/* enable clock */
	mmio_write_32(PERI_SC_PERIPH_CLKDIS3, PERI_CLK3_HKADC);
	udelay(2);
	mmio_write_32(PERI_SC_PERIPH_CLKEN3,  PERI_CLK3_HKADC);
	udelay(2);

	return;
}

int hkadc_bbp_convert(unsigned char enchan, int *pusvalue)
{
	unsigned int val = 0x00;
	unsigned int count = 0x0;
	unsigned short ret = 0;
	unsigned short rd_data0 = 0;
	unsigned short rd_data1 = 0;

	/* set channel, rate and eanble pmu hkadc */
	val = HKADC_WR01_VALUE | enchan;

	/* config operation */
	mmio_write_32(HKADC_WR01_DATA, val);
	mmio_write_32(HKADC_WR23_DATA, HKADC_WR23_VALUE);
	mmio_write_32(HKADC_WR45_DATA, HKADC_WR45_VALUE);

	/* set operation number*/
	mmio_write_32(HKADC_WR_NUM, HKADC_WR_NUM_VALUE);

	/* set delay for operations */
	mmio_write_32(HKADC_DELAY01, HKADC_DELAY01_VALUE);
	mmio_write_32(HKADC_DELAY23, HKADC_DELAY23_VALUE);

	/* enable hkadc */
	mmio_write_32(HKADC_DSP_START, 0x1);

	/* check if the enable bit has been cleared */
	do {
		if (count > HKADC_START_TIMEOUT) {
			/* disable hkadc and return error */
			mmio_write_32(HKADC_DSP_START, 0x0);
			ERROR("%s: HKADC start failed! (%d)\n", __func__);
			return -1;
		}

		val = mmio_read_32(HKADC_DSP_START);
		count++;
	} while(val & 0x1);

	/* read back adc result */
	rd_data0 = mmio_read_32(HKADC_DSP_RD2_DATA);
	rd_data1 = mmio_read_32(HKADC_DSP_RD3_DATA);

	/* combine result */
	ret = (((rd_data1 << 4) & HKADC_VALUE_HIGH) |
	       ((rd_data0 >> 4) & HKADC_VALUE_LOW));

	*pusvalue = (ret & HKADC_VALID_VALUE) * HKADC_VREF_1V8;
	*pusvalue = *pusvalue >> 12;

	INFO("[BDID] bbp convert: data=%d, count=%d, value=%d\n",
		ret, count, *pusvalue );
	return 0;
}

/*
 * Convert adcin raw data (12-bit) to remapping data (4-bit).
 */
int hkadc_adcin_data_remap(int adcin_value)
{
	unsigned int ret = BOARDID_UNKNOW;

	if (adcin_value < 0)
		ret = BOARDID_UNKNOW;
	else if (adcin_value <= BOUNDARY_VALUE0)
		ret = BOARDID_VALUE0;
	else if (adcin_value <= BOUNDARY_VALUE1)
		ret = BOARDID_VALUE1;
	else if (adcin_value <= BOUNDARY_VALUE2)
		ret = BOARDID_VALUE2;
	else if (adcin_value <= BOUNDARY_VALUE3)
		ret = BOARDID_VALUE3;
	else if (adcin_value <= BOUNDARY_VALUE4)
		ret = BOARDID_VALUE4;
	else if (adcin_value <= BOUNDARY_VALUE5)
		ret = BOARDID_VALUE5;
	else if (adcin_value <= BOUNDARY_VALUE6)
		ret = BOARDID_VALUE6;
	else if (adcin_value <= BOUNDARY_VALUE7)
		ret = BOARDID_VALUE7;
	else if (adcin_value <= BOUNDARY_VALUE8)
		ret = BOARDID_VALUE8;
	else if (adcin_value <= BOUNDARY_VALUE9)
		ret = BOARDID_VALUE9;
	else
		ret = BOARDID_UNKNOW;

	return ret;
}

int hkadc_read_board_id(unsigned int *data)
{
	int adcin0, adcin1, adcin2;
	int ret;

	/* read adc channel data */
	ret = hkadc_bbp_convert(HKADC_CHAN_BOARDID0, &adcin0);
	if (ret == -1) {
		ERROR("[BDID] failed read board id chan=%d, adc=%d\n",
			HKADC_CHAN_BOARDID0, adcin0);
		return ret;
	}
	INFO("[BDID] adcin0_V:%d\n", adcin0);

        adcin0 = hkadc_adcin_data_remap(adcin0);
        INFO("[BDID] adcin0_remap:0x%x\n", adcin0);

	ret = hkadc_bbp_convert(HKADC_CHAN_BOARDID1, &adcin1);
	if (ret == -1) {
		ERROR("[BDID] failed read board id chan=%d, adc=%d\n",
			HKADC_CHAN_BOARDID1, adcin1);
		return ret;
	}
	INFO("[BDID] adcin1_V:%d\n", adcin1);

	adcin1 = hkadc_adcin_data_remap(adcin1);
	INFO("[BDID] adcin1_remap:0x%x\n", adcin1);

	ret = hkadc_bbp_convert(HKADC_CHAN_BOARDID2, &adcin2);
	if (ret == -1) {
		ERROR("[BDID] failed read board id chan=%d, adc=%d\n",
			HKADC_CHAN_BOARDID2, adcin2);
		return ret;
	}
	INFO("[BDID] adcin2_V:%d\n", adcin2);

	adcin2 = hkadc_adcin_data_remap(adcin2);
	INFO("[BDID] adcin2_remap:0x%x\n", adcin2);

	/* Composed to adcin data to decimal boardid */
	*data = adcin2 * 100 + adcin1 * 10 + adcin0;
	INFO("[BDID] boardid: %d %d %d %d\n", *data, adcin2, adcin1, adcin0);

	return 0;
}

void init_boardid(void)
{
	unsigned int actual_boardid;
	unsigned int reg;

	hkadc_read_board_id(&actual_boardid);

	/* Set chip id to sram */
	reg = read_midr_el1();
	mmio_write_32(MEMORY_AXI_CHIP_ADDR, reg);
	INFO("[BDID] [%x] midr: 0x%x\n", MEMORY_AXI_CHIP_ADDR, reg);

	/* Set board type to sram */
	mmio_write_32(MEMORY_AXI_BOARD_TYPE_ADDR, 0x0);
	INFO("[BDID] [%x] board type: 0\n", MEMORY_AXI_BOARD_TYPE_ADDR);

	/* Set board id to sram */
	mmio_write_32(MEMORY_AXI_BOARD_ID_ADDR, 0x2b);
	INFO("[BDID] [%x] board id: 0x2b\n", MEMORY_AXI_BOARD_ID_ADDR);
	return;
}
