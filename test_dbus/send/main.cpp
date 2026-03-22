#include <dbus-c++-1/dbus-c++/dbus.h>

int main() {
  DBus::BusDispatcher dispatcher;
  DBus::default_dispatcher = &dispatcher;

  DBus::Connection dbus_conn = DBus::Connection::SystemBus();

  dbus_conn.unique_name("test.method.server");

  DBus::SignalMessage message("/test/signal/Object", "test.signal.Type",
                              "Test");
  auto writer = message.writer();
  writer.append_string("test message");

  dbus_conn.send(message);
  dbus_conn.flush();

  dbus_conn.disconnect();

  return 0;
}
