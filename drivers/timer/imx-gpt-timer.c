// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020
 * Author(s): Giulio Benetti <giulio.benetti@benettiengineering.com>
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <fdtdec.h>
#include <timer.h>
#include <dm/device_compat.h>

#include <asm/io.h>

#define GPT_CR_SWR	0x00008000

struct imx_gpt_timer_regs {
	u32 cr;
	u32 pr;
	u32 sr;
	u32 ir;
	u32 ocr1;
	u32 ocr2;
	u32 ocr3;
	u32 icr1;
	u32 icr2;
	u32 cnt;
};

struct imx_gpt_timer_priv {
	struct imx_gpt_timer_regs *base;
};

static int imx_gpt_timer_get_count(struct udevice *dev, u64 *count)
{
	struct imx_gpt_timer_priv *priv = dev_get_priv(dev);
	struct imx_gpt_timer_regs *regs = priv->base;

	*count = readl(&regs->cnt);

	return 0;
}

static int imx_gpt_timer_probe(struct udevice *dev)
{
	struct timer_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	struct imx_gpt_timer_priv *priv = dev_get_priv(dev);
	struct imx_gpt_timer_regs *regs;
	struct clk clk;
	fdt_addr_t addr;
	u32 prescaler;
	u32 rate, pr;
	int ret;

	addr = dev_read_addr(dev);
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	priv->base = (struct imx_gpt_timer_regs *)addr;

	/* TODO: Retrieve clock-source */

	/* TODO: Retrieve prescaler */

	ret = clk_get_by_index(dev, 0, &clk);
	if (ret < 0)
		return ret;

	ret = clk_enable(&clk);
	if (ret) {
		dev_err(dev, "failed to enable clock\n");
		return ret;
	}

	regs = priv->base;

	/* Reset the timer */
	setbits_le32(&regs->cr, GPT_CR_SWR);

	/* Wait for timer to finish reset */
	while(readl(&regs->cr) & GPT_CR_SWR)
		;

	/* Get timer clock rate */
	rate = clk_get_rate(&clk);

	/* we set timer prescaler to obtain a 1MHz timer counter frequency */
//	pr = (rate / CONFIG_SYS_HZ_CLOCK) - 1;
	writel(pr, &regs->pr);

	/* Set timer frequency to 1MHz */
	uc_priv->clock_rate = rate / prescaler;

	/* start timer */
//	setbits_le32(&regs->cr1, CR1_CEN);

	return 0;
}

static const struct timer_ops imx_gpt_timer_ops = {
	.get_count = imx_gpt_timer_get_count,
};

static const struct udevice_id imx_gpt_timer_ids[] = {
	{ .compatible = "fsl,imxrt-gpt" },
	{}
};

U_BOOT_DRIVER(imx_gpt_timer) = {
	.name = "imx_gpt_timer",
	.id = UCLASS_TIMER,
	.of_match = imx_gpt_timer_ids,
	.priv_auto_alloc_size = sizeof(struct imx_gpt_timer_priv),
	.probe = imx_gpt_timer_probe,
	.ops = &imx_gpt_timer_ops,
};

