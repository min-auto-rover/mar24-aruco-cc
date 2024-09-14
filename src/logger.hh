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

#define DEFAULT_FILENAME "/var/log/marvision.log"

#define log_debug (Logger::get_instance() << Logger::LogLevel::DEBUG)
#define log_info  (Logger::get_instance() << Logger::LogLevel::INFO)
#define log_warn  (Logger::get_instance() << Logger::LogLevel::WARN)
#define log_error (Logger::get_instance() << Logger::LogLevel::ERROR)
#define log_crit  (Logger::get_instance() << Logger::LogLevel::CRIT)

/**
 * @brief A singleton logger for marvision
 *
 * Use the macros to log. See documentations for `enum class LogLevel` for
 * usage.
 */
class Logger {
public:
	/**
	 * The log level.
	 */
	enum class LogLevel {
		DEBUG, ///< log using `log_debug << "message"`
		INFO,  ///< log using `log_info << "message"`
		WARN,  ///< log using `log_warn << "message"`
		ERROR, ///< log using `log_error << "message"`
		CRIT   ///< log using `log_crit << "message"`
	};
private:
	LogLevel loglevel;
	FILE* logfile;
	LogLevel curr_msg_loglevel;
	std::string print_log_level(LogLevel loglevel) const;
	Logger(const std::string& filename, LogLevel level = LogLevel::INFO);
	~Logger();
	static Logger* instance;
public:
	/**
	 * Get the instance of the singleton logger. If not exist, create the
	 * instance with the args provided.
	 */
	static Logger& get_instance(
		const std::string& filename = DEFAULT_FILENAME,
		LogLevel level = LogLevel::DEBUG);
	/**
	 * Log a message.
	 *
	 * @param message  the message to log
	 * @param level    the log level
	 */
	void log(const std::string& message, LogLevel level);
	Logger& operator<<(LogLevel level);
	Logger& operator<<(const std::string& message);
	Logger (const Logger&) = delete;
	Logger& operator=(const Logger&) = delete;
};

#endif // LOGGER_HH
