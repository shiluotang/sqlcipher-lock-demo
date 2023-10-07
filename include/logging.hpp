#ifndef LOGGING_HPP_INCLUDED
#define LOGGING_HPP_INCLUDED

#include <iomanip>
#include <sstream>
#include <ostream>
#include <iostream>
#include <vector>
#include <mutex>
#include <thread>

#include "basic_loggingstream.hpp"

namespace org {

class logging {
    public:
        virtual ~logging();

        static logging* instance();

        std::ostream* tie();

        std::ostream* tie(std::ostream *stream);

        loggingstream info();

        loggingstream warn();

        loggingstream debug();

        loggingstream error();

        std::mutex* get_mutex();
    protected:
        logging(std::ostream *stream);
    private:
        std::ostream *_M_stream;
        std::mutex _M_mutex;
};

}

#define LOG_BASE(level, expr)                       \
    do {                                            \
        org::logging::instance()->level() << expr;  \
    } while (false)

#define LOGD(expr) LOG_BASE(debug, expr)
#define LOGI(expr) LOG_BASE(info,  expr)
#define LOGW(expr) LOG_BASE(warn,  expr)
#define LOGE(expr) LOG_BASE(error, expr)

#endif // LOGGING_HPP_INCLUDED
