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

#include "logger.hh"
#include <iostream>
#include <cstdio>
#include <cerrno>
#include <cstring>
#include <ctime>

Logger* Logger::instance = nullptr;

Logger::Logger(const std::string& filename, LogLevel level)
	: loglevel(level), curr_msg_loglevel(level), logfile(nullptr) {
	logfile = fopen(filename.c_str(), "a");
	if (!logfile) {
		std::cerr << "Could not open log file: " << filename
			  << " (" << errno << ": " << strerror(errno) << ")"
			  << std::endl;
	}
}

Logger::~Logger() {
	if (logfile) {
		fclose(logfile);
	}
}

Logger&
Logger::get_instance(const std::string& filename, LogLevel level) {
	if (!instance) {
		instance = new Logger(filename, level);
	}
	return *instance;
}

std::string
Logger::print_log_level(LogLevel loglevel) const {
	switch (loglevel) {
	case LogLevel::DEBUG:
		return "DEBUG";
	case LogLevel::INFO:
		return "INFO";
	case LogLevel::WARN:
		return "WARN";
	case LogLevel::ERROR:
		return "ERROR";
	case LogLevel::CRIT:
		return "CRIT";
	}
	return nullptr;
}

void
Logger::log(const std::string& message, LogLevel level) {
	if (logfile && level >= loglevel) {
		std::time_t now = std::time(nullptr);
		char time_str[200];
		std::strftime(time_str, sizeof time_str, "%Y-%m-%d %H:%M:%S",
			      std::localtime(&now));
		fprintf(logfile, "%s [%s] \t%s\n", time_str,
			print_log_level(level).c_str(), message.c_str());
		fflush(logfile);
#ifdef LOG_COPY_STDERR
		fprintf(stderr, "%s [%s] \t%s\n", time_str,
			print_log_level(level).c_str(), message.c_str());
#endif
	}
}

Logger&
Logger::operator<<(LogLevel level) {
	curr_msg_loglevel = level;
	return *this;
}

Logger&
Logger::operator<<(const std::string& message) {
	log(message, curr_msg_loglevel);
	return *this;
}
