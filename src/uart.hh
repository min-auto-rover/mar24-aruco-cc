// SPDX-License-Identifier: GPL-3.0-or-later

/*
 * marvision -- the vision program for the minimal autonomous rover
 *
 * Copyright (C) 2024  Qiyang Sun and the MAR Project Maintainers
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef UART_HH
#define UART_HH

#include <string>

#ifndef UART_PORT
# define UART_PORT "/dev/serial0"
#endif

//#define ENABLE_OUTPUT
#define OUTPUT_TO_STDOUT

#ifndef termios_setup
# define termios_setup(tty)			\
	do {					\
		/* ---CONTROL MODE FLAGS---*/	\
		/* Disable parity */		\
		tty.c_cflag &= ~PARENB; 	\
		/* One stop bit */		\
		tty.c_cflag &= ~CSTOPB;		\
		/* Eight bits per byte */	\
		tty.c_cflag |= CS8;		\
		/* Disable RTS/CTS */		\
		tty.c_cflag &= ~CRTSCTS;	\
		/* Turn on READ ignore ctrl */	\
		tty.c_cflag |= CREAD | CLOCAL;	\
						\
		/* ---LOCAL MODE FLAGS--- */	\
		/* Disable canonical mode */	\
		tty.c_lflag &= ~ICANON;		\
		/* Disable echo */		\
		tty.c_lflag &= ~ECHO;		\
		tty.c_lflag &= ~ECHOE;		\
		tty.c_lflag &= ~ECHONL;		\
		/* Disable ISIG bit */		\
		tty.c_lflag &= ~ISIG;		\
						\
		/* ---INPUT MODE FLAGS--- */	\
		tty.c_iflag &= ~(IXON|IXOFF|IXANY);	\
		tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL); \
									\
		/* ---OUTPUT MODE FLAGS--- */				\
		tty.c_oflag &= ~OPOST;					\
		tty.c_oflag &= ~ONLCR;					\
	} while(0)
#endif

class Uart {
private:
	static bool initialised;
	static int file;
public:
	/**
	 * Initialise UART.
	 * @returns file descriptor
	 */
	static int init_uart(void);

	/**
	 * Send a char.
	 * @param msg  char to be sent
	 */
	static void send(char msg);
};

typedef Uart uart;

#endif // UART_HH
