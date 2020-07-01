/*
 *  uspispy fimrware for risc-v
 *
 *  Copyright (C) 2020 Trammell Hudson <hudson@trmm.net>
 *  Portions copyright (C) 2017  Clifford Wolf <clifford@clifford.at>
 *
 *  Permission to use, copy, modify, and/or distribute this software for any
 *  purpose with or without fee is hereby granted, provided that the above
 *  copyright notice and this permission notice appear in all copies.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 *  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 *  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 *  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 *  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 *  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */

#include <stdint.h>
#include <stdbool.h>

#  define MEM_TOTAL 0x20000 /* 128 KB */

// a pointer to this is a null pointer, but the compiler does not
// know that because "sram" is a linker symbol from sections.lds.
extern uint32_t sram;

#define reg_uart_clkdiv (*(volatile uint32_t*)0x02000004)
#define reg_uart_data (*(volatile uint32_t*)0x02000008)
#define reg_leds (*(volatile uint32_t*)0x03000000)
#define reg_uspi (*(volatile uint32_t*)0x04000000)
#define reg_wbuf (*(volatile uint32_t*)0x04100000)

typedef struct {
	volatile uint32_t counter;
	volatile uint32_t cmd;
	volatile uint32_t addr;
	volatile uint32_t len_sr;
} uspispy_t;

#define uspi ((uspispy_t*) 0x04000000)

void print_char(char c)
{
	if (c == '\n')
		print_char('\r');
	reg_uart_data = c;
}

void print(const char *p)
{
	while (*p)
		print_char(*(p++));
}

void print_hex(uint32_t v, int digits)
{
	for (int i = 7; i >= 0; i--) {
		char c = "0123456789abcdef"[(v >> (4*i)) & 15];
		if (c == '0' && i >= digits) continue;
		print_char(c);
		digits = i;
	}
}

void print_dec(uint32_t v)
{
	if (v >= 1000) {
		print(">=1000");
		return;
	}

	if      (v >= 900) { print_char('9'); v -= 900; }
	else if (v >= 800) { print_char('8'); v -= 800; }
	else if (v >= 700) { print_char('7'); v -= 700; }
	else if (v >= 600) { print_char('6'); v -= 600; }
	else if (v >= 500) { print_char('5'); v -= 500; }
	else if (v >= 400) { print_char('4'); v -= 400; }
	else if (v >= 300) { print_char('3'); v -= 300; }
	else if (v >= 200) { print_char('2'); v -= 200; }
	else if (v >= 100) { print_char('1'); v -= 100; }

	if      (v >= 90) { print_char('9'); v -= 90; }
	else if (v >= 80) { print_char('8'); v -= 80; }
	else if (v >= 70) { print_char('7'); v -= 70; }
	else if (v >= 60) { print_char('6'); v -= 60; }
	else if (v >= 50) { print_char('5'); v -= 50; }
	else if (v >= 40) { print_char('4'); v -= 40; }
	else if (v >= 30) { print_char('3'); v -= 30; }
	else if (v >= 20) { print_char('2'); v -= 20; }
	else if (v >= 10) { print_char('1'); v -= 10; }

	if      (v >= 9) { print_char('9'); v -= 9; }
	else if (v >= 8) { print_char('8'); v -= 8; }
	else if (v >= 7) { print_char('7'); v -= 7; }
	else if (v >= 6) { print_char('6'); v -= 6; }
	else if (v >= 5) { print_char('5'); v -= 5; }
	else if (v >= 4) { print_char('4'); v -= 4; }
	else if (v >= 3) { print_char('3'); v -= 3; }
	else if (v >= 2) { print_char('2'); v -= 2; }
	else if (v >= 1) { print_char('1'); v -= 1; }
	else print_char('0');
}

static inline uint32_t rdcycle(void)
{
	uint32_t cycles;
	__asm__ volatile ("rdcycle %0" : "=r"(cycles));
	return cycles;
}


void main()
{
	reg_leds = 31;
	//reg_uart_clkdiv = 104; // 115200 baud = 12 MHz / 104
	reg_uart_clkdiv = 139; // 115200 baud = 16 MHz / 139

/*
	while(1)
	{
		print_char('a');
		reg_leds = ~reg_leds;
	}
*/

	uint32_t last_report = 0;
	uint32_t last_cmd = 0;
	while(1)
	{
		const uint32_t now = rdcycle();
		if (now - last_report > 16000000) {
			last_report = now;
			print_hex(uspi->counter, 8);
			print("---\n");
		}

		const uint32_t cmd = uspi->cmd;
		if (cmd == last_cmd)
			continue;

		print_hex(cmd >> 8, 6);
		print_char(' ');
		print_hex(cmd & 0xFF, 2);
		print_char(' ');
		print_hex(uspi->addr, 6);
		print_char(' ');
		print_hex(uspi->len_sr >> 8, 4);
		print_char('\n');
		last_cmd = cmd;
		last_report = now;
	}
}