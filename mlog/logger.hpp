/*
 * logger.hpp
 *
 *  Created on: Aug 9, 2012
 *      Author: philipp
 */

#ifndef __LOGGER_HPP__
#define __LOGGER_HPP__

#include <string>
#if defined(_GLIBCXX_HAS_GTHREADS) || defined(__clang__) ||                    \
    (defined(_MSC_VER) && (_MSC_VER >= 1700))
#include <thread>
#define THREAD_GET_ID() std::this_thread::get_id()
#else
#include <boost/thread.hpp>

namespace std {
using boost::mutex;
using boost::recursive_mutex;
using boost::lock_guard;
using boost::condition_variable;
using boost::unique_lock;
using boost::thread;
}
#define THREAD_GET_ID() boost::this_thread::get_id()
#endif
#include <chrono>
#include <boost/format.hpp>
#include "manager.hpp"

enum mlog_level {
	trace,
	debug,
	info,
	warning,
	error,
	fatal
};

namespace mlog {

extern mlog_manager *manager;

template <typename T> inline std::string level_to_string(T &&level) {
	if (level == mlog_level::trace)
		return "trace";
	else if (level == mlog_level::debug)
		return "debug";
	else if (level == mlog_level::info)
		return "info";
	else if (level == mlog_level::warning)
		return "warning";
	else if (level == mlog_level::error)
		return "error";
	else
		return "fatal";
}

struct log_position {
	log_position() {}

	log_position(std::string _filename, std::size_t _line_number)
	    : filename(std::move(_filename)),
	      line_number(std::move(_line_number)) {}

	std::string filename;
	std::size_t line_number;
};

struct log_metadata {

	// typedef std::chrono::high_resolution_clock clocks;
	typedef std::chrono::system_clock clocks;

	bool use_time;
	bool use_thread_id;
	bool use_position;
	mlog_level level;
	short session_id;
	std::chrono::time_point<clocks> time;
	std::thread::id thread_id;
	log_position position;
	bool m_use_position;

	log_metadata()
	    : use_time(true), use_thread_id(false), use_position(false),
	      level(info), session_id(0) {}

	log_metadata(mlog_level &&lvl, short session_id, bool _use_time,
		     bool _use_thread_id);

	log_metadata(mlog_level &&lvl, short session_id, bool _use_time,
		     bool _use_thread_id, log_position &&position,
		     bool _use_position);
	log_metadata(mlog_level &&lvl, short session_id, bool _use_time,
		     bool _use_thread_id, const log_position &position,
		     bool _use_position);

	std::string to_string(const std::string& end_string = std::string(), bool end_line = false) const;

	std::ostream &output(std::ostream &stream) const;
};

struct logger_base {
    
	logger_base();

	virtual ~logger_base();

	inline void write(mlog_level &&level, boost::format &&format,
			  log_position &&pos) {
		log_metadata metadata(std::move(level), mlog::manager->session(), mlog::manager->use_time(),
				      mlog::manager->use_thread_id(), std::move(pos),
				      mlog::manager->use_position());
		write_to_log(std::move(metadata), boost::str(format));
	}

	inline void write(mlog_level &&level, const boost::format &format,
			  log_position &&pos) {
		log_metadata metadata(std::move(level), mlog::manager->session(), mlog::manager->use_time(),
				       mlog::manager->use_thread_id(), std::move(pos),
				      mlog::manager->use_position());
		write_to_log(std::move(metadata), boost::str(format));
	}

	inline void write(mlog_level &&level, std::string &&log_text,
			  log_position &&pos) {
		log_metadata metadata(std::move(level), mlog::manager->session(), mlog::manager->use_time(),
				      mlog::manager->use_thread_id(), std::move(pos),
				      mlog::manager->use_position());
		write_to_log(std::move(metadata), std::move(log_text));
	}

	template <typename T>
	void write(mlog_level &&level, const std::string &log_text, T &&pos) {
		log_metadata metadata(std::move(level), mlog::manager->session(), mlog::manager->use_time(),
				       mlog::manager->use_thread_id(), std::forward<T>(pos),
				      mlog::manager->use_position());
		write_to_log(std::move(metadata), std::string(log_text));
	}

	// virtual void flush() = 0;
	virtual void write_to_log(log_metadata &&metadata,
				  std::string &&log_text) = 0;

	virtual void write_to_log(const log_metadata& metadata,
				  const std::string& log_text) = 0;


};

template<typename T>
struct logger : logger_base {

	virtual void write_to_log(log_metadata &&metadata,
				  std::string &&log_text) {
		static_cast<T*>(this)->write_to_log(std::move(metadata), std::move(log_text));
	}

	virtual void write_to_log(const log_metadata& metadata,
				  const std::string& log_text) {
		static_cast<T*>(this)->write_to_log(metadata, log_text);
	}
};

}
#endif /* LOGGER_HPP_ */

#ifdef MLOG_NO_LIB
#include "impl/logger.hpp"
#endif
