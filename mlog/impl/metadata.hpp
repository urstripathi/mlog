/*
*
*	Author: Philipp Zschoche
*
*/

#ifndef __METADATA_IPP__
#define __METADATA_IPP__

#include "../metadata.hpp"
#include <cstdio>
#if _MSC_VER
#define snprintf _snprintf_s
#endif

#include <sstream>

namespace mlog {


std::string log_metadata::to_string(const std::string &end_string,
				    bool end_line) const {

	static const std::size_t max_size = 1024;
	std::string result;
	result.resize(max_size);

	char *buffer = const_cast<char *>(result.c_str());

	if (manager->use_time()) {

		std::time_t timet = clocks::to_time_t(time);

		unsigned long long ms =
		    std::chrono::duration_cast<std::chrono::nanoseconds>(
			time.time_since_epoch()).count() -
		    timet * 1000000000;

		std::tm *tm = std::gmtime(&timet);
		static int current_hour = -1;
		static int local_factor = 0;

		if (tm->tm_hour != current_hour) {
			current_hour = tm->tm_hour;
			tm = std::localtime(&timet);
			local_factor = tm->tm_hour - current_hour;
		} else {
			tm->tm_hour += local_factor;
		}

		if (manager->use_thread_id()) {
			if (position.has_value()) // 2012-11-02 15:24:04.345
						  // [file:line_number
				// 24-0x7fff72ca8180]{warning}:
				snprintf(
				    buffer, max_size,
				    "%04i-%02i-%02i %02i:%02i:%02i.%llu "
				    "[%s:%li %02i-%s]{%s}: ",
				    1900 + tm->tm_year, tm->tm_mon, tm->tm_mday,
				    tm->tm_hour, tm->tm_min, tm->tm_sec, ms,
				    position.filename.c_str(),
				    position.line_number, manager->session(),
				    thread_id_to_string(thread_id).c_str(),
				    level_to_string(level).c_str());
			else // 2012-11-02 15:24:04.345
				// [24-0x7fff72ca8180]{warning}:
				snprintf(buffer, max_size,
					 "%04i-%02i-%02i %02i:%02i:%02i.%llu "
					 "[%02i-%s]{%s}: ",
					 1900 + tm->tm_year, tm->tm_mon,
					 tm->tm_mday, tm->tm_hour, tm->tm_min,
					 tm->tm_sec, ms, manager->session(),
					 thread_id_to_string(thread_id).c_str(),
					 level_to_string(level).c_str());
		} else // 2012-11-02 15:24:04.345 [24]{warning}:
		{
			if (position.has_value())
				snprintf(
				    buffer, max_size,
				    "%04i-%02i-%02i %02i:%02i:%02i.%llu[%s:%li "
				    "%02i]{%s}: ",
				    1900 + tm->tm_year, tm->tm_mon, tm->tm_mday,
				    tm->tm_hour, tm->tm_min, tm->tm_sec, ms,
				    position.filename.c_str(),
				    position.line_number, manager->session(),
				    level_to_string(level).c_str());
			else
				snprintf(buffer, max_size,
					 "%04i-%02i-%02i "
					 "%02i:%02i:%02i.%llu[%02i]{%s}: ",
					 1900 + tm->tm_year, tm->tm_mon,
					 tm->tm_mday, tm->tm_hour, tm->tm_min,
					 tm->tm_sec, ms, manager->session(),
					 level_to_string(level).c_str());
		}
	} else if (manager->use_thread_id()) //[24-0x7fff72ca8180]{warning}:
	{
		if (position.has_value())
			snprintf(buffer, max_size, "[%s:%li %02i-%s]{%s}: ",
				 position.filename.c_str(),
				 position.line_number, manager->session(),
				 thread_id_to_string(thread_id).c_str(),
				 level_to_string(level).c_str());
		else
			snprintf(buffer, max_size, "[%02i-%s]{%s}: ",
				 manager->session(),
				 thread_id_to_string(thread_id).c_str(),
				 level_to_string(level).c_str());
	} else if (position.has_value()) {
		snprintf(buffer, max_size, "[%s:%li %02i]{%s}: ",
			 position.filename.c_str(), position.line_number,
			 manager->session(), level_to_string(level).c_str());
	} else //[24]{warning}:
	{
		snprintf(buffer, max_size, "[%02i]{%s}: ", manager->session(),
			 level_to_string(level).c_str());
	}
	std::size_t len = strlen(buffer);
	if (!end_string.empty()) {
		memcpy(&buffer[len], end_string.c_str(), end_string.size());
		len += end_string.size();
	}

	if (end_line) {
		result.resize(len + 2);
		result[len] = '\r';
		result[len + 1] = '\n';
	} else {
		result.resize(len);
	}
	return result;
}

} /* mlog */

#endif