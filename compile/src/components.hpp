#include <html/components.hpp>

struct MapH {
  MapH(std::string const &so_name);
  ~MapH();
  components::Map const *map;
  components::ServerMap const *server_map;

  components::Component const *find(char const *name) const;

  static void hot_reload();

private:
  void *so;
  std::string lib_full_name;

  friend void init();
};
