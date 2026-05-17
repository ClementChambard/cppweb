#pragma once

#include <cassert>
#include <defines.hpp>
#include <map>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace cppweb {

/**
 * @struct ServerRenderingContext
 *
 * This struct contains the data used for server side rendering
 */
struct ServerRenderingContext {
  /**
   * @field ServerRenderingContext::current_page : std::string
   *
   * The name of the page that has been requested by the browser
   */
  std::string current_page;

  /**
   * @field ServerRenderingContext::page_params :
   * std::unordered_map<std::string, std::string>
   *
   * The route parameters for the current page
   *
   * @example
   *
   * page: /test/[some_param]/test2/[some_param2]
   * request: /test/123/test2/file.png
   *
   * page_params: {
   *   "some_param": "123",
   *   "some_param2": "file.png"
   * }
   *
   * @endexample
   */
  std::unordered_map<std::string, std::string> page_params;

  /**
   * @field ServerRenderingContext::query_params :
   * std::unordered_map<std::string, std::string>
   *
   * The query parameters parsed at the end of the url
   */
  std::unordered_map<std::string, std::string> query_params;

  /**
   * @field ServerRenderingContext::response_type : std::string
   * @default "text/html"
   *
   * The type of the response that will be set in the 'Content-Type' http
   * header. By default, this corresponds to an html web page
   */
  std::string response_type = "text/html";

  /**
   * @field ServerRenderingContext::user_data : void *
   * @default nullptr
   *
   * The developper's own server side rendering context data
   */
  void *user_data = nullptr;

  /**
   * @method ServerRenderingContext::redirect : void (std::string const &)
   *
   * The server side rendering will stop, and the user will be redirected to
   * some other page instead.
   *
   * @param loc the url to redirect to
   */
  void redirect(std::string const &loc) {
    m_redirect = true;
    m_param_str = loc;
  }

  /**
   * @method ServerRenderingContext::set_error : void (std::string const &)
   *
   * The server side rendering will stop, and an error message will be displayed
   * instead
   *
   * @param err the error to show to the user
   */
  void set_error(std::string const &err) {
    m_error = true;
    m_param_str = err;
  }

  /**
   * @method ServerRenderingContext::custom_response :
   * void (std::string const &, u8 const &, u64)
   *
   * Replace the regular cppweb response with some custom content
   *
   * @param response_type the type of the response to put in 'Content-Type'
   * header
   * @param data the buffer containing the raw data to send to the user
   * @param size the size of the data to send to the user
   */
  void custom_response(std::string const &response_type, u8 const *data,
                       u64 size);

  /**
   * @constmethod ServerRenderingContext::get_cookie :
   * std::optional<std::string> (std::string const &)
   *
   * Gets a cookie from the request
   *
   * @param name the name of the cookie to get
   * @return the cookie or std::nullopt if not set
   */
  std::optional<std::string> get_cookie(std::string const &name) const;

  /**
   * @method ServerRenderingContext::get<T> : T & ()
   *
   * Allows the developper to get their own rendering context
   *
   * @return the user data
   */
  template <typename T> T &get() {
    assert(user_data != nullptr);
    return *static_cast<T *>(user_data);
  }

  /**
   * @method ServerRenderingContext::get_body : std::vector<u8> (std::string &)
   *
   * Computes the body of the response either from the rendered page or from a
   * custom response
   *
   * @param s the rendered page
   * @return the actual body of the response
   */
  std::vector<u8> get_body(std::string &s);

  /**
   * @method ServerRenderingContext::set_cookies : void (std::string_view)
   *
   * Allows to give the context the cookies of the request
   *
   * @param cookies_str the cookies as sent in the 'Cookie' header
   */
  void set_cookies(std::string_view cookies_str);

private:
  std::map<std::string, std::string> m_cookies;
  std::string m_param_str;
  std::vector<u8> m_custom_response;
  bool m_error = false;
  bool m_redirect = false;
  bool m_is_custom_response = false;
};

} // namespace cppweb
