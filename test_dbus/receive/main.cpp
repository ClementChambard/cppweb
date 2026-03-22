#include <dbus-c++-1/dbus-c++/dbus.h>
#include <dbus-c++-1/dbus-c++/eventloop.h>
#include <dbus-c++-1/dbus-c++/message.h>

int main() {
  DBus::BusDispatcher dispatcher;
  DBus::default_dispatcher = &dispatcher;

  DBus::Connection dbus_conn = DBus::Connection::SystemBus();

  dbus_conn.unique_name("test.method.receiver");

  dbus_conn.add_match("type='signal',interface='test.signal.Type'");
  dbus_conn.flush();

  return 0;
}
