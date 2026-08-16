/*
 * Allwinner Sun60iw2 do poweroff in uboot with arisc
 *
 * (C) Copyright 2021  <xinouyang@allwinnertech.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <sunxi_board.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <private_uboot.h>

#define UARTLCR	0xC
#define UARTHALT 0xA4
#define UARTLCR_DLAB	0x80 /* bit7 */
#define UARTDLL 0x0
#define UARTDLLM 0x4
#define UART_MEM_SIZE	0x1000
#define UARTHALT_AT_BUSY 0x2

int sunxi_baud_rate_get(void)
{
	uint32_t val;
	static int32_t sclk = -1;
	static int32_t baudrate = -1;
	uint32_t  uart_base = 0;
	uint32_t clk_src = 0, factor_m = 0, divisor = 0;

	uart_base = (SUNXI_UART0_BASE + uboot_spare_head.boot_data.uart_port * UART_MEM_SIZE);

	val = readl(uart_base + UARTHALT);
	val |= UARTHALT_AT_BUSY;
	writel(val, uart_base + UARTHALT);

	/*adaptive select sclk and baudrate*/
	val = readl(SUNXI_UART_CLK_REG);
	clk_src = (val >> 24) & 0x7;
	factor_m = (val & 0x1f) + 1;

	val = readl(uart_base + UARTLCR);
	val |= UARTLCR_DLAB;
	writel(val, uart_base + UARTLCR);

	divisor = (readl(uart_base + UARTDLL) & 0xFF) | (readl(uart_base + UARTDLLM) & 0xFF) << 8;
	val &= ~UARTLCR_DLAB;
	writel(val, uart_base + UARTLCR);

	val = readl(uart_base + UARTHALT);
	val &= ~UARTHALT_AT_BUSY;
	writel(val, uart_base + UARTHALT);

	if (clk_src == SUNXI_UART_CLK_SRC_SEL_HOSC) {
		sclk = 24000000 / factor_m;
		baudrate = sclk / (16 * divisor);
		if (baudrate == 115384)
			baudrate = 115200;
	} else if (clk_src == SUNXI_UART_CLK_SRC_PERI0_600M) {
		sclk = 600000000 / factor_m;
		baudrate = sclk / (16 * divisor);
	} else if (clk_src == SUNXI_UART_CLK_SRC_PERI0_480M) {
		sclk = 480000000 / factor_m;
		baudrate = sclk / (16 * divisor);
	}

	return baudrate;
}
