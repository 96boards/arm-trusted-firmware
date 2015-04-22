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

#include <assert.h>
#include <arch_helpers.h>
#include <arm_gic.h>
#include <debug.h>
#include <cci400.h>
#include <errno.h>
#include <gic_v2.h>
#include <hi6220.h>
#include <mmio.h>
#include <platform.h>
#include <platform_def.h>
#include <psci.h>
#include "hikey_def.h"
#include "hikey_private.h"

static void hikey_power_on_cpu(int cluster, int cpu, int linear_id)
{
	unsigned int data, expected;

	/* check if cpu has been powered on */
	if (!(mmio_read_32(ACPU_SC_CPUx_PW_ISO_STAT(linear_id)) & CPU_PW_ISO))
		return;

	/* Set arm64 mode */
	data = mmio_read_32(ACPU_SC_CPUx_CTRL(linear_id));
	data |= CPU_CTRL_AARCH64_MODE;
	mmio_write_32(ACPU_SC_CPUx_CTRL(linear_id), data);

	/* Enable debug module */
	data = mmio_read_32(ACPU_SC_PDBGUP_MBIST);
	if (cluster)
		expected = 1 << (cpu + PDBGUP_CLUSTER1_SHIFT);
	else
		expected = 1 << cpu;
	mmio_write_32(ACPU_SC_PDBGUP_MBIST, data | expected);
	do {
		/* RAW barrier */
		data = mmio_read_32(ACPU_SC_PDBGUP_MBIST);
	} while (!(data & expected));

	/* Assert reset state */
	mmio_write_32(ACPU_SC_CPUx_RSTEN(linear_id), 0x1f);

	mmio_write_32(ACPU_SC_CPUx_MTCMOS_EN(linear_id), CPU_MTCMOS_PW);

	do {
		data = mmio_read_32(ACPU_SC_CPUx_MTCMOS_TIMER_STAT(linear_id));
	} while (!(data & CPU_MTCMOS_TIMER_STA));

	mmio_write_32(ACPU_SC_CPUx_CLKEN(linear_id), 0x5);
	do {
		data = mmio_read_32(ACPU_SC_CPUx_CLK_STAT(linear_id));
	} while ((data & 5) != 5);
	mmio_write_32(ACPU_SC_CPUx_CLKEN(linear_id), CPU_MTCMOS_TIMER_STA);
	do {
		data = mmio_read_32(ACPU_SC_CPUx_CLK_STAT(linear_id));
	} while (!(data & CPU_MTCMOS_TIMER_STA));

	mmio_write_32(ACPU_SC_CPUx_PW_ISODIS(linear_id), CPU_PW_ISO);
	do {
		data = mmio_read_32(ACPU_SC_CPUx_PW_ISO_STAT(linear_id));
	} while (data & CPU_PW_ISO);

	/* Release reset state */
	mmio_write_32(ACPU_SC_CPUx_RSTDIS(linear_id), 0x1f);
	return;
}

static void hikey_power_on_cluster(int cluster)
{
	unsigned int data, temp;

	if (cluster)
		data = PW_ISO_A53_1_EN;
	else
		data = PW_ISO_A53_0_EN;

	/* the cluster has been powered on yet */
	if (!(mmio_read_32(ACPU_SC_A53_CLUSTER_ISO_STA) & data))
		return;

	/* Set timer stable interval */
	mmio_write_32(ACPU_SC_A53_x_MTCMOS_TIMER(cluster), 0xff);

	/* Assert cluster reset */
	if (cluster)
		data = SRST_CLUSTER1;
	else
		data = SRST_CLUSTER0;
	mmio_write_32(ACPU_SC_RSTEN, data);
	do {
		temp = mmio_read_32(ACPU_SC_RST_STAT);
	} while ((temp & data) != data);

	if (cluster)
		data = PW_MTCMOS_EN_A53_1_EN;
	else
		data = PW_MTCMOS_EN_A53_0_EN;
	mmio_write_32(ACPU_SC_A53_CLUSTER_MTCMOS_EN, data);
	do {
		temp = mmio_read_32(ACPU_SC_A53_CLUSTER_MTCMOS_STA);
	} while ((temp & data) != data);

	if (cluster)
		data = HPM_L2_1_CLKEN;
	else
		data = HPM_L2_CLKEN;
	mmio_write_32(ACPU_SC_CLKEN, data);
	do {
		temp = mmio_read_32(ACPU_SC_CLK_STAT);
	} while ((temp & data) != data);

	if (cluster)
		data = G_CPU_1_CLKEN;
	else
		data = G_CPU_CLKEN;
	mmio_write_32(ACPU_SC_CLKEN, data);
	do {
		temp = mmio_read_32(ACPU_SC_CLK_STAT);
	} while ((temp & data) != data);

	if (cluster)
		data = PW_ISO_A53_1_EN;
	else
		data = PW_ISO_A53_0_EN;
	mmio_write_32(ACPU_SC_A53_CLUSTER_ISO_DIS, data);
	do {
		temp = mmio_read_32(ACPU_SC_A53_CLUSTER_ISO_STA);
	} while (temp & data);

	/* Release cluster reset */
	if (cluster)
		data = SRST_CLUSTER1;
	else
		data = SRST_CLUSTER0;
	mmio_write_32(ACPU_SC_RSTDIS, data);
	do {
		temp = mmio_read_32(ACPU_SC_RST_STAT);
	} while (data & temp);

	return;
}

/*******************************************************************************
 * Hikey handler called when an affinity instance is about to be turned on. The
 * level and mpidr determine the affinity instance.
 ******************************************************************************/
int32_t hikey_affinst_on(uint64_t mpidr,
			 uint64_t sec_entrypoint,
			 uint32_t afflvl,
			 uint32_t state)
{
	int cpu, cluster;
	unsigned long linear_id;
	unsigned int reg;

	linear_id = platform_get_core_pos(mpidr);
	cluster = (mpidr & MPIDR_CLUSTER_MASK) >> MPIDR_AFF1_SHIFT;
	cpu = mpidr & MPIDR_CPU_MASK;

	VERBOSE("#%s, mpidr:%llx, afflvl:%x, state:%x\n", __func__, mpidr, afflvl, state);

	/* directly return for power on */
	if (state == PSCI_STATE_ON)
		return PSCI_E_SUCCESS;

	switch (afflvl) {
	case MPIDR_AFFLVL0:
		psci_program_mailbox(mpidr, sec_entrypoint);

		/* Setup cpu entrypoint when it next powers up */
		mmio_write_32(ACPU_SC_CPUx_RVBARADDR(linear_id),
			      (unsigned int)(sec_entrypoint >> 2));

		hikey_power_on_cpu(cluster, cpu, linear_id);

		reg = ((0x1 << linear_id) << 16) | (0x1<<15);
		gicd_write_sgir(GICD_BASE, reg);
		dsb();

		reg = ((0x1 << linear_id) << 16);
		gicd_write_sgir(GICD_BASE, reg);
		dsb();

		break;

	case MPIDR_AFFLVL1:
		hikey_power_on_cluster(cluster);
		break;
	}

	return PSCI_E_SUCCESS;
}

/*******************************************************************************
 * Hikey handler called when an affinity instance has just been powered on after
 * being turned off earlier. The level and mpidr determine the affinity
 * instance. The 'state' arg. allows the platform to decide whether the cluster
 * was turned off prior to wakeup and do what's necessary to setup it up
 * correctly.
 ******************************************************************************/
void hikey_affinst_on_finish(uint32_t afflvl, uint32_t state)
{
	unsigned long mpidr;
	unsigned long linear_id;

	/* Get the mpidr for this cpu */
	mpidr = read_mpidr_el1();
	linear_id = platform_get_core_pos(mpidr);

	/*
	 * Perform the common cluster specific operations i.e enable coherency
	 * if this cluster was off.
	 */
	if (afflvl != MPIDR_AFFLVL0)
		cci_enable_cluster_coherency(mpidr);

	psci_program_mailbox(mpidr, 0x0);

	/* Cleanup cpu entry point */
	mmio_write_32(ACPU_SC_CPUx_RVBARADDR(linear_id), 0x0);

	/* Enable the gic cpu interface */
	arm_gic_cpuif_setup();

	/* TODO: if GIC in AON, then just need init for cold boot */
	arm_gic_pcpu_distif_setup();
}

static int32_t hikey_do_plat_actions(uint32_t afflvl, uint32_t state)
{
	uint32_t max_phys_off_afflvl;

	assert(afflvl <= MPIDR_AFFLVL1);

	if (state != PSCI_STATE_OFF)
		return -EAGAIN;

	/*
	 * Find the highest affinity level which will be suspended and postpone
	 * all the platform specific actions until that level is hit.
	 */
	max_phys_off_afflvl = psci_get_max_phys_off_afflvl();
	assert(max_phys_off_afflvl != PSCI_INVALID_DATA);
	assert(psci_get_suspend_afflvl() >= max_phys_off_afflvl);
	if (afflvl != max_phys_off_afflvl)
		return -EAGAIN;

	return 0;
}

static void hikey_affinst_off(uint32_t afflvl, uint32_t state)
{
	if (hikey_do_plat_actions(afflvl, state) == -EAGAIN)
		return;

	if (afflvl != MPIDR_AFFLVL0)
		cci_disable_cluster_coherency(read_mpidr_el1());

	return;
}

static void __dead2 hikey_system_reset(void)
{
	VERBOSE("%s: reset system\n", __func__);

	/* Send the system reset request */
	mmio_write_32(AO_SC_SYS_STAT0, 0x48698284);

	wfi();
	panic();
}

/*******************************************************************************
 * Export the platform handlers to enable psci to invoke them
 ******************************************************************************/
static const plat_pm_ops_t hikey_ops = {
	.affinst_on		= hikey_affinst_on,
	.affinst_on_finish	= hikey_affinst_on_finish,
	.affinst_off		= hikey_affinst_off,
	.affinst_standby	= NULL,
	.affinst_suspend	= NULL,
	.affinst_suspend_finish	= NULL,
	.system_off		= NULL,
	.system_reset		= hikey_system_reset,
};

/*******************************************************************************
 * Export the platform specific power ops.
 ******************************************************************************/
int32_t platform_setup_pm(const plat_pm_ops_t **plat_ops)
{
	*plat_ops = &hikey_ops;
	return 0;
}
