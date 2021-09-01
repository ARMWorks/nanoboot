/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2021 Jeff Kent <jeff@jkent.net>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; version 2.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "asm/io.h"
#include "s5pv210.h"
#include "uart.h"

#include <stdint.h>
#include <math.h>


uint16_t divslot_table[] = {
	0x0000, 0x0080, 0x0808, 0x0888, 0x2222, 0x4924, 0x4A52, 0x54AA,
	0x5555, 0xD555, 0xD5D5, 0xDDD5, 0xDDDD, 0xDFDD, 0xDFDF, 0xFFDF,
};

#define PCLK 				66700000

void uart0_init(void)
{
	uint32_t gpio_base = ELFIN_GPIO_BASE;
	uint32_t uart_base = ELFIN_UART_BASE + ELFIN_UART0_OFFSET;

	writel(0x22222222, gpio_base + GPA0CON_OFFSET);
	writel(0x00002222, gpio_base + GPA1CON_OFFSET);
	writel(0x00000001, uart_base + UFCON_OFFSET);
	writel(0x00000000, uart_base + UMCON_OFFSET);
	writel(0x00000003, uart_base + ULCON_OFFSET);
	writel(0x00000005, uart_base + UCON_OFFSET);

	uart0_set_baudrate(115200);
}

void uart0_set_baudrate(uint32_t baud)
{
	uint32_t uart_base = ELFIN_UART_BASE + ELFIN_UART0_OFFSET;

	uint32_t brdiv = (uint64_t)PCLK * 100 / (baud * 16);
	writel((brdiv / 100) - 1, uart_base + UBRDIV_OFFSET);
	writel((((brdiv % 100) * 16) + 50) / 100, uart_base + UDIVSLOT_OFFSET);
}

int uart0_getc(void)
{
	uint32_t uart_base = ELFIN_UART_BASE + ELFIN_UART0_OFFSET;

	while (!(readl(uart_base + UTRSTAT_OFFSET) & UTRSTAT_RX_READY));
	return readl(uart_base + URXH_OFFSET);
}

int uart0_putc(int c)
{
	uint32_t uart_base = ELFIN_UART_BASE + ELFIN_UART0_OFFSET;

	while (!(readl(uart_base + UTRSTAT_OFFSET) & UTRSTAT_TX_EMPTY));
	writel((char) c, uart_base + UTXH_OFFSET);
    return c;
}
