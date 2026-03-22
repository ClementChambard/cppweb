#include <html/components.hpp>

struct MapH {
  MapH(std::string const &so_name);
  ~MapH();
  components::Map const *map;

  components::Component const *find(char const *name) const;

private:
  void *so;
};
