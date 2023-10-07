#ifndef BASIC_LOGGINGSTREAM_HPP_INCLUDED
#define BASIC_LOGGINGSTREAM_HPP_INCLUDED

#include <ostream>
#include <sstream>
#include <string>

namespace org {

template <
    typename CharT,
    typename Traits = std::char_traits<CharT>
        >
class basic_loggingstream
    : public std::basic_ostream<CharT, Traits> {
    private:
        typedef std::basic_ostream<CharT, Traits> super_type;
        typedef basic_loggingstream<CharT, Traits> this_type;
        typedef std::basic_stringbuf<CharT, Traits> buffer_type;
    public:
        basic_loggingstream(
                std::string const &name,
                std::ostream *stream,
                std::mutex *mutex)
            : super_type(NULL)
            , _M_buf(new buffer_type())
            , _M_name(name)
            , _M_stream(stream)
            , _M_mutex(mutex)
        {
            super_type::rdbuf(_M_buf);
        }

        basic_loggingstream(this_type const &other)
            : _M_buf(NULL)
            , _M_name(other._M_name)
            , _M_stream(other._M_stream)
            , _M_mutex(other._M_mutex)
        {
            std::swap(_M_buf, other._M_buf);
        }

        virtual ~basic_loggingstream() {
            print();
        }
    protected:
    private:
        std::string timestamp() {
            time_t now = std::time(NULL);
            if (now == static_cast<time_t>(-1))
                throw std::runtime_error("std::time");
            struct tm tm;
            // FIXME non standard localtime_r
            if (!localtime_r(&now, &tm))
                throw std::runtime_error("localtime_r");
            std::vector<char> buffer(30, 0);
            if (!std::strftime(
                        &buffer[0],
                        buffer.size(),
                        "%Y-%m-%d %H:%M:%S %z",
                        &tm))
                throw std::runtime_error("strftime");
            return &buffer[0];
        }

        void print() {
            if (!_M_buf || !_M_stream)
                return;
            std::ostringstream oss;
            oss << timestamp()
                << " " << _M_name
                << " " << std::this_thread::get_id()
                << " " <<  _M_buf << std::endl;
            std::ostream &out = *_M_stream;
            if (!_M_mutex) {
                out << oss.str() << std::flush;
                return;
            }
            std::unique_lock<std::mutex> locker(*_M_mutex);
            out << oss.str() << std::flush;
        }

        mutable buffer_type *_M_buf;
        std::string _M_name;
        std::ostream *_M_stream;
        std::mutex *_M_mutex;
};

typedef basic_loggingstream<char> loggingstream;
typedef basic_loggingstream<wchar_t> wloggingstream;

} // namespace org

#endif  // BASIC_LOGGINGSTREAM_HPP_INCLUDED
