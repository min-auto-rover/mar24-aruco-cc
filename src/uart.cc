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

#include <cstdlib>
#include <fcntl.h>
#include <string>
#include <termios.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <cstring>

#include "logger.hh"
#include "uart.hh"

bool Uart::initialised = false;
int Uart::file = -1;

int
Uart::init_uart(void) {
	int fd = -1;
#ifdef ENABLE_OUTPUT
	fd = open(UART_PORT, O_RDWR | O_NOCTTY | O_NDELAY); // FIXME RdWr or Wonly
	if (fd < 0) {
		log_crit << "Could not open UART, see below for errno";
		log_crit << std::to_string(errno);
		log_warn << "Errno printed, Quitting now";
		exit(EXIT_FAILURE);
	}
	struct termios options;
	tcgetattr(fd, &options);
	termios_setup(options); //macro
	cfsetspeed(&options, B9600);
	tcflush(uart_fd, TCIFLUSH);
	if (tcsetattr(fd, TCSANOW, &options) != 0) {
		log_crit << "Could not setup serial, see below for errno";
		log_crit << std::to_string(errno);
		log_warn << "Errno printed, Quitting now";
		exit(EXIT_FAILURE);
	}
#endif
#ifdef OUTPUT_TO_STDOUT
	fd = open("/dev/stdout", O_WRONLY);
#endif
	file = fd;
	initialised = true;
	log_info << "output init done";
	return fd;
}

void
Uart::send(char msg) {
	// TODO
#ifdef ENABLE_OUTPUT
	if (!initialised) {
		log_crit << "Output before initialised, quitting";
		exit(EXIT_FAILURE);
	}
#endif
#if defined(ENABLE_OUTPUT) || defined(OUTPUT_TO_STDOUT)
	if (file == -1) {
		log_crit << "Output file is -1";
	}
	char to_write[1] = {msg};
	if (write(file, to_write, sizeof(to_write)) == -1) {
		log_crit << "Output written size is -1, write error:";
		log_crit << std::to_string(errno);
		log_warn << "Errno printed";
	}
#endif
	return;
}
