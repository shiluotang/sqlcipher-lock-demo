#include <iostream>

#include "basic_loggingstream.hpp"
#include "logging.hpp"

namespace org {

logging::logging(std::ostream *stream)
    : _M_stream(stream)
{
}

logging::~logging() {
    _M_stream = NULL;
}

logging* logging::instance() {
    static struct logging_initializer {
        logging_initializer(std::ostream *stream)
            :_M_logger(stream)
        {
        }

        logging _M_logger;
        // std::cin is tied to std::cout
    } initialization(std::cin.tie());
    return &initialization._M_logger;
}

std::ostream* logging::tie() {
    return _M_stream;
}

std::ostream* logging::tie(std::ostream *stream) {
    std::ostream *orig = _M_stream;
    _M_stream = stream;
    return orig;
}

loggingstream logging::debug() {
    return loggingstream("D", _M_stream, &_M_mutex);
}

loggingstream logging::info() {
    return loggingstream("I", _M_stream, &_M_mutex);
}

loggingstream logging::warn() {
    return loggingstream("W", _M_stream, &_M_mutex);
}

loggingstream logging::error() {
    return loggingstream("E", _M_stream, &_M_mutex);
}

}  // namespace org
