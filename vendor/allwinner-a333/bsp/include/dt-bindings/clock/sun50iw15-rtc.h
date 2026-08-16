// SPDX-License-Identifier: (GPL-2.0+ or MIT)

#ifndef _DT_BINDINGS_CLK_SUN50IW15_RTC_H_
#define _DT_BINDINGS_CLK_SUN50IW15_RTC_H_


#define CLK_DCXO24M_OUT		0
#define CLK_IOSC		1
#define CLK_IOSC_DIV32K		2
#define CLK_OSC32K		3
#define CLK_DCXO24M_DIV32K	4
#define CLK_RTC32K		5
#define CLK_RTC_1K		6
#define CLK_OSC32K_OUT		7
#define CLK_RTC_SPI		8

#define CLK_RTC_MAX_NO		(CLK_RTC_SPI + 1)

#endif /* _DT_BINDINGS_CLK_SUN50IW15_RTC_H_ */
