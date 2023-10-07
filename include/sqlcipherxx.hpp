#ifndef SQLCIPHERXX_HPP_INCLUDED
#define SQLCIPHERXX_HPP_INCLUDED

#include <memory>
#include <string>

#include <sqlite3.h>

namespace org {

class sqlcipherxx {
public:
    class statement {
        public:
            statement();
            virtual ~statement();

            bool execute();
            bool next();

            int ncols();
            std::string colname(int icol);
            bool is_null(int icol);
            std::string get_string(int icol, bool *null = NULL);
            double get_double(int icol, bool *null = NULL);
            void set_string(int icol, std::string const&);
            void set_double(int icol, double const&);
            void set_null(int icol);

            std::string sql() const;
            std::string expanded_sql() const;
            void throws(int ecode, std::string const&);
        protected:
            explicit statement(sqlite3_stmt*);
        private:
            sqlite3_stmt *_M_stmt;
            friend class sqlcipherxx;
    };

    class mutex {
        public:
            virtual ~mutex();

            bool try_lock();
            void lock();
            void unlock();

            void swap(mutex&);
        protected:
            explicit mutex(sqlite3_mutex*, bool);
        private:
            sqlite3_mutex *_M_mutex;
            bool _M_own;

            mutex(mutex const&);
            mutex& operator=(mutex const&);
            friend class sqlcipherxx;
    };

    class transaction {
        public:
            virtual ~transaction();
            void commit();
            void rollback();
        protected:
            explicit transaction(sqlcipherxx&);
        private:
            sqlcipherxx &_M_s;
            bool _M_completed;
            friend class sqlcipherxx;
    };

    sqlcipherxx();
    sqlcipherxx(
            std::string const &filename,
            int flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
            std::string const &vfs = "");
    virtual ~sqlcipherxx();
    operator bool() const;

    sqlcipherxx& open(
            std::string const& filename,
            int flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
            std::string const &vfs = "");
    void close();
    void execute(std::string const &sql);
    std::shared_ptr<transaction> begin_transaction();
    std::shared_ptr<transaction> begin_deferred();
    std::shared_ptr<transaction> begin_exclusive();
    std::shared_ptr<transaction> begin_immediate();
    std::shared_ptr<statement> prepare(std::string const&);

    static int is_threadsafe();
    void lock();
    void unlock();
    bool try_lock();

    std::string db_filename() const;

    int limit(int category);
    int limit(int category, int value);
    void set_extended_errcode(bool);

    void throws(int ecode, std::string const &message);
protected:
    std::shared_ptr<mutex> get_mutex();
private:
    sqlite3 *_M_db;

    sqlcipherxx(sqlcipherxx const&);
    sqlcipherxx& operator=(sqlcipherxx const&);
};

}

#endif // SQLCIPHERXX_HPP_INCLUDED
