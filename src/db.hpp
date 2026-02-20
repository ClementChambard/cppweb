#ifndef IG_DB_HPP
#define IG_DB_HPP

#include <algorithm>
#include <defines.hpp>
#include <mutex>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>
#include <type_traits>

std::string db_open_file(char const *file_name, i32 &fd);
void db_write_file(i32 fd, char const *data, u64 size);
void db_close_file(i32 fd);
std::string_view db_next_line(std::string_view &sv);

struct Item {
  i32 id;
};
template <typename T>
concept Item_c = std::is_base_of_v<Item, T>;

template <Item_c T> struct Db {
  Db(char const *file_name) {
    auto s = db_open_file(file_name, fd);
    auto sv = std::string_view(s);
    u32 count = std::stoi(std::string(db_next_line(sv)));
    next_id = std::stoi(std::string(db_next_line(sv)));
    items.resize(count);
    for (u32 i = 0; i < count; i++) {
      items[i].id = std::stoi(std::string(db_next_line(sv)));
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

  i32 fd;
  i32 next_id;

  std::vector<T> items;

  static inline Db<T> *instance = nullptr;
  std::mutex instance_mutex;
  static void lock() { instance->instance_mutex.lock(); }
  static void unlock() { instance->instance_mutex.unlock(); }

  bool del_id(i32 idx) {
    auto it = std::find_if(items.begin(), items.end(), [idx](T const &item) { return item.id == idx; });
    if (it == items.end()) return false;
    items.erase(it);
    return true;
  }

  T* get_id(i32 idx) {
    auto it = std::find_if(items.begin(), items.end(), [idx](T const &item) { return item.id == idx; });
    if (it == items.end()) return nullptr;
    return &*it;
  }

  T& add_new() {
    items.resize(items.size() + 1);
    auto &b = items.back();
    b.id = next_id++;
    return b;
  }

  void write() {
    std::ostringstream oss;
    oss << i32(items.size()) << '\n' << i32(next_id) << '\n';
    for (auto const &i : items) {
      oss << i32(i.id) << '\n';
      i.write(oss);
    }
    std::string out = oss.str();
    db_write_file(fd, out.c_str(), out.size());
  }
};
#define DECL_DB(ty) template <> Db<ty> *Db<ty>::instance;

struct Sondage : Item {
  std::string name;
  std::string button_text;
  std::string desc;
  std::string route;
  bool active;
  void read(std::string_view &);
  void write(std::ostringstream &) const;
};

struct Rdv : Item {
  u32 heure;
  u32 minute;
  std::string eleve;
  std::string txt;
  void read(std::string_view &);
  void write(std::ostringstream &) const;
};

struct Rollers : Item {
  std::string eleve;
  int size;
  bool has_roller;
  bool has_helmet;
  bool has_protect;
  bool has_answered;
  void read(std::string_view &);
  void write(std::ostringstream &) const;
};

struct Piscine : Item {
  std::string date;
  std::string parent;
  void read(std::string_view &);
  void write(std::ostringstream &) const;
};

using SondagesDb = Db<Sondage>;
using RdvsDb = Db<Rdv>;
using RollersDb = Db<Rollers>;
using PiscineDb = Db<Piscine>;

#define ITER_DB(item, id, db)                                                  \
  for (int i = (db::lock(), 0), id = 0; i < 1; db::unlock(), i++)              \
    for (auto s = db::get().items.begin(); s != db::get().items.end();         \
         s++, id++)

#endif // !IG_DB_HPP
