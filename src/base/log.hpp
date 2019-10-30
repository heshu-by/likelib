#pragma once

#include <boost/log/trivial.hpp>

#include <cstddef>

namespace base
{

enum class LogLevel
{
    ALL,
    DEBUG,
    INFO,
    WARNING,
    ERROR
};


namespace Sink
{
    static constexpr const std::size_t DISABLE = 0;
    static constexpr const std::size_t STDOUT = 1;
    static constexpr const std::size_t FILE = 2;
} // namespace Sink


void initLog(LogLevel logLevel = LogLevel::ALL, std::size_t mode = Sink::FILE);
} // namespace base

#define LOG_TRACE BOOST_LOG_TRIVIAL(trace)
#define LOG_DEBUG BOOST_LOG_TRIVIAL(debug)
#define LOG_INFO BOOST_LOG_TRIVIAL(info)
#define LOG_WARNING BOOST_LOG_TRIVIAL(warning)
#define LOG_ERROR BOOST_LOG_TRIVIAL(error)
#define LOG_FATAL BOOST_LOG_TRIVIAL(fatal)