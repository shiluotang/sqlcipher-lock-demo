#include <cstdlib>
#include <ctime>

#include <iostream>
#include <fstream>
#include <ostream>
#include <sstream>
#include <string>
#include <chrono>
#include <atomic>
#include <thread>
#include <condition_variable>
#include <functional>
#include <utility>

#include <gtest/gtest.h>

#include "logging.hpp"
#include "sqlcipherxx.hpp"

namespace {

void print_record(
        std::shared_ptr<org::sqlcipherxx::statement> stmt) {
    int ncols = stmt->ncols();
    int irow = 0;
    while (stmt->next()) {
        std::ostringstream oss;
        oss << stmt->colname(0) << " = " << stmt->get_string(0);
        for (int i = 1, n = ncols; i < n; ++i)
            oss << ", " << stmt->colname(i)
                << " = " << stmt->get_string(i);
        LOGI("row(" << irow << "): " << oss.str());
        ++irow;
    }
}

class execute_with_input_connection {
    public:
        void operator() (
                std::atomic_bool &terminated,
                org::sqlcipherxx &sqlcipher,
                std::string const &sql) {
            typedef org::sqlcipherxx sqlcipherxx;
            typedef sqlcipherxx::statement statement;

            sqlcipherxx &s = sqlcipher;
            while (!terminated) {
                std::shared_ptr<statement> stmt = s.prepare(sql);
                try {
                if (stmt->execute())
                    print_record(stmt);
                } catch (std::exception const &e) {
                    LOGW("[c++ exception] " << e.what());
                    throw;
                }
            }
        }
};

class execute_with_spawned_connection {
    public:
        void operator() (
                std::atomic_bool &terminated,
                std::string const &filename,
                std::string const &sql) {
            typedef org::sqlcipherxx sqlcipherxx;
            typedef sqlcipherxx::statement statement;
            typedef sqlcipherxx::transaction transaction;

            while (!terminated) {
                sqlcipherxx s;
                s.open(filename);
                s.set_extended_errcode(true);
                LOGI(s.db_filename());
                try {
                std::shared_ptr<transaction> tran = s.begin_deferred();
                std::shared_ptr<statement> stmt = s.prepare(sql);
                bool has_results = stmt->execute();
                tran->commit();
                if (has_results)
                    print_record(stmt);
                } catch (std::exception const &e) {
                    LOGW("[c++ exception] " << e.what());
                    throw;
                }
            }
        }
};
}

TEST(LockTest, SingleConnection) {
    using std::chrono::duration_cast;
    using std::chrono::hours;
    using std::chrono::seconds;
    using std::chrono::minutes;
    using org::sqlcipherxx;

    std::ofstream outfile("debug.log");
    org::logging::instance()->tie(&outfile);

    sqlcipherxx sqlcipher;

    LOGI("sqlcipherxx::is_threadsafe() = " << sqlcipherxx::is_threadsafe());
    sqlcipher.open("a.db");
#define LOG_LIMIT(name) LOGI(#name << " = " << sqlcipher.limit(name))
    LOG_LIMIT(SQLITE_LIMIT_LENGTH);
    LOG_LIMIT(SQLITE_LIMIT_SQL_LENGTH);
    LOG_LIMIT(SQLITE_LIMIT_COLUMN);
    LOG_LIMIT(SQLITE_LIMIT_EXPR_DEPTH);
    LOG_LIMIT(SQLITE_LIMIT_COMPOUND_SELECT);
    LOG_LIMIT(SQLITE_LIMIT_VDBE_OP);
    LOG_LIMIT(SQLITE_LIMIT_FUNCTION_ARG);
    LOG_LIMIT(SQLITE_LIMIT_ATTACHED);
    LOG_LIMIT(SQLITE_LIMIT_LIKE_PATTERN_LENGTH);
    LOG_LIMIT(SQLITE_LIMIT_VARIABLE_NUMBER);
    LOG_LIMIT(SQLITE_LIMIT_TRIGGER_DEPTH);
    LOG_LIMIT(SQLITE_LIMIT_WORKER_THREADS);
#undef LOG_LIMIT

    sqlcipher.execute("CREATE TABLE IF NOT EXISTS "
            "student(id INTEGER PRIMARY KEY, sno INTEGER, sname STRING)");
    std::vector<std::thread> threads;
    std::atomic<bool> terminated(false);
    execute_with_input_connection dst;
    int ndeleters = 1;
    int ninserters = 1;
    int nreaders = 1;
    for (int i = 0, n = ndeleters; i < n; ++i)
        threads.push_back(
                std::thread(
                    dst,
                    std::ref(terminated),
                    std::ref(sqlcipher),
                    "DELETE FROM student WHERE id < (SELECT MAX(id) FROM student)"));
    for (int i = 0, n = ninserters; i < n; ++i)
        threads.push_back(
                std::thread(
                    dst,
                    std::ref(terminated),
                    std::ref(sqlcipher),
                    "INSERT INTO student(sno, sname) VALUES(1, 'SQG')"));
    for (int i = 0, n = nreaders; i < n; ++i)
        threads.push_back(
                std::thread(
                    dst,
                    std::ref(terminated),
                    std::ref(sqlcipher),
                    "SELECT * FROM student"));

    minutes d(1);
    seconds sec = duration_cast<seconds>(d);
    LOGI("sleep_for " << sec.count() << " second(s)");
    std::this_thread::sleep_for(d);
    LOGI("terminated = true");
    terminated = true;
    for (int i = 0, n = threads.size(); i < n; ++i)
        if (threads[i].joinable())
            threads[i].join();
}

TEST(LockTest, MultipleConnections) {
    using std::chrono::duration_cast;
    using std::chrono::hours;
    using std::chrono::seconds;
    using std::chrono::minutes;
    using org::sqlcipherxx;

    std::ofstream outfile("debug.log");
    org::logging::instance()->tie(&outfile);

    sqlcipherxx sqlcipher;
    std::string filename = "a.db";
    LOGI("sqlcipherxx::is_threadsafe() = " << sqlcipherxx::is_threadsafe());
    sqlcipher.open(filename);
    sqlcipher.execute("PRAGMA journal_mode=WAL;");
#define LOG_LIMIT(name) LOGI(#name << " = " << sqlcipher.limit(name))
    LOG_LIMIT(SQLITE_LIMIT_LENGTH);
    LOG_LIMIT(SQLITE_LIMIT_SQL_LENGTH);
    LOG_LIMIT(SQLITE_LIMIT_COLUMN);
    LOG_LIMIT(SQLITE_LIMIT_EXPR_DEPTH);
    LOG_LIMIT(SQLITE_LIMIT_COMPOUND_SELECT);
    LOG_LIMIT(SQLITE_LIMIT_VDBE_OP);
    LOG_LIMIT(SQLITE_LIMIT_FUNCTION_ARG);
    LOG_LIMIT(SQLITE_LIMIT_ATTACHED);
    LOG_LIMIT(SQLITE_LIMIT_LIKE_PATTERN_LENGTH);
    LOG_LIMIT(SQLITE_LIMIT_VARIABLE_NUMBER);
    LOG_LIMIT(SQLITE_LIMIT_TRIGGER_DEPTH);
    LOG_LIMIT(SQLITE_LIMIT_WORKER_THREADS);
#undef LOG_LIMIT
    sqlcipher.execute("CREATE TABLE IF NOT EXISTS "
            "student(id INTEGER PRIMARY KEY, sno INTEGER, sname STRING)");
    sqlcipher.close();
    std::vector<std::thread> threads;
    std::atomic<bool> terminated(false);
    execute_with_spawned_connection dst;
    int ndeleters = 1;
    int ninserters = 1;
    int nreaders = 1;
    for (int i = 0, n = ndeleters; i < n; ++i)
        threads.push_back(
                std::thread(
                    dst,
                    std::ref(terminated),
                    filename,
                    "DELETE FROM student WHERE id < (SELECT MAX(id) FROM student)"));
    for (int i = 0, n = ninserters; i < n; ++i)
        threads.push_back(
                std::thread(
                    dst,
                    std::ref(terminated),
                    filename,
                    "INSERT INTO student(sno, sname) VALUES(1, 'SQG')"));
    for (int i = 0, n = nreaders; i < n; ++i)
        threads.push_back(
                std::thread(
                    dst,
                    std::ref(terminated),
                    filename,
                    "SELECT * FROM student"));

    minutes d(1);
    seconds sec = duration_cast<seconds>(d);
    LOGI("sleep_for " << sec.count() << " second(s)");
    std::this_thread::sleep_for(d);
    LOGI("terminated = true");
    terminated = true;
    for (int i = 0, n = threads.size(); i < n; ++i)
        if (threads[i].joinable())
            threads[i].join();
    std::cout << std::this_thread::get_id();
}
