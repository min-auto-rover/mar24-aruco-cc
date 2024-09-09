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

#ifndef LOGGER_HH
#define LOGGER_HH

#include <iostream>
#include <string>

class Logger {
public:
	enum class LogLevel {
		DEBUG,
		INFO,
		WARN,
		ERROR,
		CRIT
	};
private:
	LogLevel loglevel;
	FILE* logfile;
	LogLevel curr_msg_loglevel;
	std::string print_log_level(LogLevel loglevel) const;
public:
	Logger(const std::string& filename, LogLevel level = LogLevel::INFO);
	~Logger();
	void log(const std::string& message, LogLevel level);
	Logger& operator<<(LogLevel level);
	Logger& operator<<(const std::string& message);
};

#endif // LOGGER_HH
