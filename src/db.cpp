#include "db.hpp"
#include "fcntl.h"
#include "unistd.h"
#include <sstream>
#include <string>
#include <string_view>

std::string db_open_file(char const *file_name, i32 &fd) {
  fd = open(file_name, O_RDWR);
  u64 size = lseek(fd, 0, SEEK_END);
  lseek(fd, 0, SEEK_SET);
  std::string data(size, '\0');
  read(fd, data.data(), data.size());
  return data;
}

void db_write_file(i32 fd, const char *data, u64 size) {
  lseek(fd, 0, SEEK_SET);
  ftruncate(fd, 0);
  write(fd, data, size);
}

void db_close_file(i32 fd) { close(fd); }

std::string_view db_next_line(std::string_view &sv) {
  u64 pos = sv.find('\n');
  if (pos == std::string_view::npos) {
    auto r = sv;
    sv = "";
    return r;
  }
  auto r = sv.substr(0, pos);
  sv = sv.substr(pos + 1);
  return r;
}

void Sondage::read(std::string_view &sv) {
  name = std::string(db_next_line(sv));
  button_text = std::string(db_next_line(sv));
  active = std::stoi(std::string(db_next_line(sv)));
  desc = std::string(db_next_line(sv));
  route = std::string(db_next_line(sv));
}

void Sondage::write(std::ostringstream &out) const {
  out << name << '\n'
      << button_text << '\n'
      << i32(active) << '\n'
      << desc << '\n'
      << route << '\n';
}

void Rdv::read(std::string_view &sv) {
  heure = std::stoi(std::string(db_next_line(sv)));
  minute = std::stoi(std::string(db_next_line(sv)));
  eleve = std::string(db_next_line(sv));
  txt = std::string(db_next_line(sv));
}

void Rdv::write(std::ostringstream &out) const {
  out << heure << '\n' << minute << '\n' << eleve << '\n' << txt << '\n';
}

void Rollers::read(std::string_view &sv) {
  eleve = std::string(db_next_line(sv));
  size = std::stoi(std::string(db_next_line(sv)));
  has_roller = bool(std::stoi(std::string(db_next_line(sv))));
  has_helmet = bool(std::stoi(std::string(db_next_line(sv))));
  has_protect = bool(std::stoi(std::string(db_next_line(sv))));
  has_answered = bool(std::stoi(std::string(db_next_line(sv))));
}

void Rollers::write(std::ostringstream &out) const {
  out << eleve << '\n'
      << size << '\n'
      << i32(has_roller) << '\n'
      << i32(has_helmet) << '\n'
      << i32(has_protect) << '\n'
      << i32(has_answered) << '\n';
}

void Piscine::read(std::string_view &sv) {
  date = std::string(db_next_line(sv));
  parent = std::string(db_next_line(sv));
}

void Piscine::write(std::ostringstream &out) const {
  out << date << '\n' << parent << '\n';
}

std::string get_sondage_desc(char const *route) {
  SondagesDb::lock();
  std::string info;
  for (auto const &s : SondagesDb::get().items) {
    if (s.route == route) {
      info = s.desc;
      break;
    }
  }
  SondagesDb::unlock();
  return info;
}
