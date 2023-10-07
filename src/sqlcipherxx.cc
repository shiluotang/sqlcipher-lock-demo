#include <iomanip>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <memory>

#include <sqlite3.h>

#include "sqlcipherxx.hpp"

namespace {
class errors {
    public:
        static void throws(int ecode, std::string const &msg) {
            throw std::runtime_error(message(ecode, msg));
        }

        static std::string message(
                int ecode,
                std::string const &msg) {
            char const *p = ::sqlite3_errstr(ecode);
            std::ostringstream es;
            if (!p)
                es << msg;
            else
                es << p;
            es << " (" << ecode << ")";
            return es.str();
        }
};
}

namespace org {

sqlcipherxx::sqlcipherxx() :_M_db(NULL) {
}

sqlcipherxx::~sqlcipherxx() {
    this->close();
}

sqlcipherxx::operator bool() const {
    return _M_db != NULL;
}

sqlcipherxx::sqlcipherxx(
        std::string const &filename,
        int flags,
        std::string const &vfs) {
    open(filename, flags, vfs);
}

sqlcipherxx& sqlcipherxx::open(
        std::string const &filename,
        int flags,
        std::string const &vfs) {
    char const *zVfs = NULL;
    if (!vfs.empty())
        zVfs = vfs.c_str();
    int rc = ::sqlite3_open_v2(filename.c_str(), &_M_db, flags, zVfs);
    if (rc != SQLITE_OK)
        errors::throws(rc, "sqlite3_open_v2");
    return *this;
}

void sqlcipherxx::close() {
    if (_M_db) {
        int rc = ::sqlite3_close(_M_db);
        if (rc != SQLITE_OK)
            throws(rc, "sqlite3_close");
        _M_db = NULL;
    }
}

void sqlcipherxx::execute(std::string const &sql) {
    char *errmsg = NULL;
    int rc = ::sqlite3_exec(_M_db, sql.c_str(), NULL, NULL, &errmsg);
    if (rc != SQLITE_OK) {
        std::string message;
        if (errmsg) {
            message.assign(errmsg);
            ::sqlite3_free(errmsg);
            errmsg = NULL;
            throw std::runtime_error(message);
        }
        throws(rc, "sqlite3_exec");
    }
}

std::shared_ptr<sqlcipherxx::transaction>
sqlcipherxx::begin_transaction() {
    this->execute("BEGIN TRANSACTION");
    return std::shared_ptr<transaction>(new transaction(*this));
}

std::shared_ptr<sqlcipherxx::transaction>
sqlcipherxx::begin_deferred() {
    this->execute("BEGIN DEFERRED");
    return std::shared_ptr<transaction>(new transaction(*this));
}

std::shared_ptr<sqlcipherxx::transaction>
sqlcipherxx::begin_exclusive() {
    this->execute("BEGIN EXCLUSIVE");
    return std::shared_ptr<transaction>(new transaction(*this));
}

std::shared_ptr<sqlcipherxx::transaction>
sqlcipherxx::begin_immediate() {
    this->execute("BEGIN IMMEDIATE");
    return std::shared_ptr<transaction>(new transaction(*this));
}

std::shared_ptr<sqlcipherxx::statement>
sqlcipherxx::prepare(std::string const &sql) {
    sqlite3_stmt *stmt = NULL;
    int rc = ::sqlite3_prepare_v2(
            _M_db,
            sql.c_str(), sql.length(),
            &stmt,
            NULL);
    if (rc != SQLITE_OK)
        throws(rc, "sqlite3_prepare_v2");
    return std::shared_ptr<statement>(new statement(stmt));
}

int sqlcipherxx::is_threadsafe() {
    return sqlite3_threadsafe();
}

sqlcipherxx::statement::statement()
    :_M_stmt(NULL)
{
}

sqlcipherxx::statement::statement(sqlite3_stmt *stmt)
    :_M_stmt(stmt)
{
}

sqlcipherxx::statement::~statement() {
    if (_M_stmt) {
        int rc = ::sqlite3_finalize(_M_stmt);
        if (rc != SQLITE_OK)
            throws(rc, "sqlite3_finalize");
    }
}

bool sqlcipherxx::statement::execute() {
    int rc = SQLITE_OK;
    bool is_query = true;
    rc = sqlite3_step(_M_stmt);
    if (rc != SQLITE_OK
            && rc != SQLITE_ROW
            && rc != SQLITE_DONE)
        throws(rc, "sqlite3_step");
    is_query = this->ncols() > 0;
    rc = sqlite3_reset(_M_stmt);
    if (rc != SQLITE_OK)
        throws(rc, "sqlite3_reset");
    return is_query;
}

bool sqlcipherxx::statement::next() {
    int rc = sqlite3_step(_M_stmt);
    if (rc != SQLITE_ROW
            && rc != SQLITE_DONE)
        throws(rc, "sqlite3_step");
    return rc == SQLITE_ROW;
}

int sqlcipherxx::statement::ncols() {
    return sqlite3_column_count(_M_stmt);
}

std::string sqlcipherxx::statement::colname(int icol) {
    char const *p = sqlite3_column_name(_M_stmt, icol);
    if (!p)
        throw std::runtime_error("sqlite3_column_name");
    return std::string(p);
}

std::string sqlcipherxx::statement::get_string(int icol, bool *null) {
    unsigned char const* p = sqlite3_column_text(_M_stmt, icol);
    if (!p)
        throw std::runtime_error("sqlite3_column_text");
    if (null)
        *null = this->is_null(icol);
    return std::string(reinterpret_cast<char const*>(p));
}

double sqlcipherxx::statement::get_double(int icol, bool *null) {
    double d = sqlite3_column_double(_M_stmt, icol);
    if (null)
        *null = this->is_null(icol);
    return d;
}

void sqlcipherxx::statement::set_string(int iparam, std::string const& value) {
    int rc = sqlite3_bind_text(
            _M_stmt,
            iparam,
            value.c_str(),
            value.length(),
            NULL);
    if (rc != SQLITE_OK)
        throws(rc, "sqlite3_bind_text");
}

void sqlcipherxx::statement::set_double(int iparam, double const& value) {
    int rc = sqlite3_bind_double(
            _M_stmt,
            iparam,
            value);
    if (rc != SQLITE_OK)
        throws(rc, "sqlite3_bind_double");
}

void sqlcipherxx::statement::set_null(int iparam) {
    int rc = sqlite3_bind_null(
            _M_stmt,
            iparam);
    if (rc != SQLITE_OK)
        throws(rc, "sqlite3_bind_null");
}

bool sqlcipherxx::statement::is_null(int icol) {
    return sqlite3_column_type(_M_stmt, icol) == SQLITE_NULL;
}


std::string sqlcipherxx::statement::sql() const {
    char const *s = sqlite3_sql(_M_stmt);
    if (!s)
        throw std::runtime_error("sqlite3_sql");
    return std::string(s);
}

std::string sqlcipherxx::statement::expanded_sql() const {
    char const *s = sqlite3_expanded_sql(_M_stmt);
    if (!s)
        throw std::runtime_error("sqlite3_sql");
    return std::string(s);
}

void sqlcipherxx::statement::throws(
        int ecode,
        std::string const& message) {
    std::ostringstream es;
    es << errors::message(ecode, message) << ": " << this->expanded_sql();
    throw std::runtime_error(es.str());
}

sqlcipherxx::mutex::mutex(sqlite3_mutex* mutex, bool own)
    : _M_mutex(mutex)
    , _M_own(own)
{
}

sqlcipherxx::mutex::~mutex() {
    if (_M_mutex) {
        unlock();
        if (_M_own)
            sqlite3_mutex_free(_M_mutex);
        _M_mutex = NULL;
    }
}

void sqlcipherxx::mutex::lock() {
    sqlite3_mutex_enter(_M_mutex);
}

bool sqlcipherxx::mutex::try_lock() {
    int rc = sqlite3_mutex_try(_M_mutex);
    if (rc != SQLITE_OK
            && rc != SQLITE_BUSY)
        throw std::runtime_error("sqlite3_mutex_try");
    return rc == SQLITE_OK;
}

void sqlcipherxx::mutex::unlock() {
    sqlite3_mutex_leave(_M_mutex);
}

sqlcipherxx::transaction::transaction(sqlcipherxx &s)
    : _M_s(s)
    , _M_completed(false)
{
}

sqlcipherxx::transaction::~transaction() {
    try {
        commit();
    } catch (...) {
        rollback();
    }
}

void sqlcipherxx::transaction::commit() {
    if (!_M_completed)
        _M_s.execute("COMMIT");
    _M_completed = true;
}

void sqlcipherxx::transaction::rollback() {
    if (!_M_completed)
        _M_s.execute("ROLLBACK");
    _M_completed = true;
}

void sqlcipherxx::lock() {
    return get_mutex()->lock();
}

void sqlcipherxx::unlock() {
    return get_mutex()->unlock();
}

bool sqlcipherxx::try_lock() {
    return get_mutex()->try_lock();
}

std::string sqlcipherxx::db_filename() const {
    char const *p = sqlite3_db_filename(_M_db, NULL);
    return p ? p : "";
}

int sqlcipherxx::limit(int category) {
    return sqlite3_limit(_M_db, category, -1);
}

int sqlcipherxx::limit(int category, int value) {
    return sqlite3_limit(_M_db, category, value);
}

void sqlcipherxx::set_extended_errcode(bool on) {
    int flag = on ? 1 : 0;
    int rc = sqlite3_extended_result_codes(_M_db, flag);
    if (SQLITE_OK != rc)
        throws(rc, "sqlite3_extended_result_codes");
}

void sqlcipherxx::throws(int ecode, std::string const &message) {
    std::ostringstream es;
    es << errors::message(ecode, message) << ": " << db_filename();
    throw std::runtime_error(es.str());
}

std::shared_ptr<sqlcipherxx::mutex>
sqlcipherxx::get_mutex() {
    sqlite3_mutex *m = sqlite3_db_mutex(_M_db);
    return std::shared_ptr<mutex>(new mutex(m, false));
}

}  // namespace org
