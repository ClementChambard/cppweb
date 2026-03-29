#pragma once

#include <defines.hpp>
#include <functional>
#include <iostream>
#include <map>
#include <sqlite3.h>
#include <string>
#include <vector>

using Rows = std::vector<std::map<std::string, std::string>>;

struct QueryExecutor {
  sqlite3 *m_db;
  std::string m_query;
  bool m_has_run = false;
  Rows m_output;
  using ThenFn = std::function<void(Rows &&)>;
  ThenFn m_then = nullptr;
  bool m_has_then = false;
  QueryExecutor(sqlite3 *db, std::string const &query)
      : m_db(db), m_query(query) {}
  ~QueryExecutor() { run(); }

  static int query_callback(void *self, int argc, char **argv,
                            char **azColName) {
    QueryExecutor *executor = static_cast<QueryExecutor *>(self);
    if (!executor)
      return 1;
    if (!executor->m_has_then)
      return 0;
    executor->m_output.emplace_back();
    std::map<std::string, std::string> &out = executor->m_output.back();
    for (int i = 0; i < argc; i++) {
      if (argv[i] == nullptr)
        argv[i] = const_cast<char *>("<NULL>");
      out.insert(
          std::make_pair<std::string, std::string>(azColName[i], argv[i]));
    }
    return 0;
  }

  QueryExecutor &then(ThenFn f) {
    m_then = f;
    m_has_then = true;
    return *this;
  }
  void run() {
    if (m_has_run)
      return;
    m_has_run = true;

    char *zErrMsg = 0;
    int rc =
        sqlite3_exec(m_db, m_query.c_str(), query_callback, this, &zErrMsg);
    if (rc != SQLITE_OK) {
      std::cerr << "SQL error: " << zErrMsg << '\n';
      sqlite3_free(zErrMsg);
    }
    if (m_has_then)
      m_then(std::move(m_output));
  }
};

struct Database {
  Database(std::string const &file_name) {
    if (file_name != "")
      init(file_name);
  }

  void init(std::string const &file_name) {
    int rc = sqlite3_open(file_name.c_str(), &m_db);
    if (rc) {
      std::cerr << "Can't open database: " << file_name << '\n';
      std::exit(EXIT_FAILURE);
    }
  }

  void cleanup() { sqlite3_close(m_db); }

  // CreateTable create_table(std::string const &name) { return
  // CreateTable{this, name}; } InsertInto insert_into(std::string const &name)
  // { return InsertInto{this, name}; } DropTable drop_table(std::string const
  // &name) { return DropTable{this, name}; }

  QueryExecutor exec(std::string const &query) {
    // std::cout << "Executing query: " << query << '\n';
    return QueryExecutor{m_db, query};
  }

  ~Database() { cleanup(); }

private:
  sqlite3 *m_db;
};
