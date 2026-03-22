#include <api/builder.hpp>

auto api_endpoint = api::Endpoint()
                        .get("/hello",
                             [](api::Context &c) {
                               c.res.body("text/plain", "Hello, world");
                               return c.ok();
                             })
                        .get("/world",
                             [](api::Context &c) {
                               c.res.body("text/plain", "ZA WARUDO");
                               return c.ok();
                             })
                        .register_at_root("/");
