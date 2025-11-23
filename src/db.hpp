#ifndef IG_DB_HPP
#define IG_DB_HPP

#include "html/page.hpp"
#include <defines.hpp>
#include <mutex>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

std::string db_open_file(char const *file_name, i32 &fd);
void db_write_file(i32 fd, char const *data, u64 size);
void db_close_file(i32 fd);
std::string_view db_next_line(std::string_view &sv);

template <typename T> struct Db {
  Db(char const *file_name) {
    auto s = db_open_file(file_name, fd);
    auto sv = std::string_view(s);
    u32 count = std::stoi(std::string(db_next_line(sv)));
    items.resize(count);
    for (u32 i = 0; i < count; i++) {
      items[i].read(sv);
    }
    instance = this;
  }
  ~Db() {
    write();
    db_close_file(fd);
    instance = nullptr;
  }

  static Db<T> &get() { return *instance; }

  void add_listener(html::Page &p) {
    write_listeners.push_back(&p.needs_rebuild);
  }

  i32 fd;

  std::vector<T> items;

  std::vector<bool *> write_listeners;

  static inline Db<T> *instance = nullptr;
  std::mutex instance_mutex;
  static void lock() { instance->instance_mutex.lock(); }
  static void unlock() { instance->instance_mutex.unlock(); }

  void write() {
    std::ostringstream oss;
    oss << i32(items.size()) << '\n';
    for (auto const &i : items) {
      i.write(oss);
    }
    std::string out = oss.str();
    db_write_file(fd, out.c_str(), out.size());
    for (auto b : write_listeners)
      *b = true;
  }
};
#define DECL_DB(ty) template <> Db<ty> *Db<ty>::instance;

struct Sondage {
  std::string name;
  std::string button_text;
  std::string desc;
  std::string route;
  bool active;
  void read(std::string_view &);
  void write(std::ostringstream &) const;
};

struct Rdv {
  u32 heure;
  u32 minute;
  std::string eleve;
  std::string txt;
  void read(std::string_view &);
  void write(std::ostringstream &) const;
};

struct Rollers {
  std::string eleve;
  int size;
  bool has_roller;
  bool has_helmet;
  bool has_protect;
  bool has_answered;
  void read(std::string_view &);
  void write(std::ostringstream &) const;
};

struct Piscine {
  std::string date;
  std::string parent;
  void read(std::string_view &);
  void write(std::ostringstream &) const;
};

using SondagesDb = Db<Sondage>;
using RdvsDb = Db<Rdv>;
using RollersDb = Db<Rollers>;
using PiscineDb = Db<Piscine>;

std::string get_sondage_desc(char const *route);

#define ITER_DB(item, id, db)                                                  \
  for (int i = (db::lock(), 0), id = 0; i < 1; db::unlock(), i++)              \
    for (auto s = db::get().items.begin(); s != db::get().items.end();         \
         s++, id++)

#endif // !IG_DB_HPP
