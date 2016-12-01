#pragma once

#include <boost/optional.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/variant.hpp>
#include <spine/Value.h>
#include "StoredQueryConfig.h"
#include "WfsConvenience.h"

namespace SmartMet
{
namespace Plugin
{
namespace WFS
{
class RequestParameterMap;
class SupportsExtraHandlerParams;

/**
 *   @brief Parameter template item represents content of scalar parameter template
 *          or template of array parameter element
 *
 *   Examples:
 *   - <b>${}</b> - absent parameter
 *   - <b>foo</b> - simple text
 *   - <b>${foo}</b> - strong reference to request parameter. The actual parameter to which
 *           it refers must actually be declared in configuration
 *   - <b>${foo[3]}</b> - strong reference to request array parameter element
 *   - <b>${foo : FOO}</b> - strong reference with default value
 *   - <b>${foo > defaultFoo}</b> - similar but parameter <b>defaultFoo</b> is used if the value
 *           of parameter <b>foo</b> is missing
 *   - <b>%{foo}</b> - weak reference to parameter. No initialization time checks for parameter
 *           declaration is performed. Weak parameter can be used if actual parameter values are
 *           being generated instead of providing them directly in the request
 */
struct ParameterTemplateItem
{
  bool optional;
  bool absent;
  bool weak;
  boost::shared_ptr<SmartMet::Spine::Value> plain_text;
  boost::optional<std::string> param_ref;
  boost::optional<std::size_t MAY_ALIAS> param_ind;
  boost::optional<std::string> default_value;
  boost::optional<std::string> redirect_name;

 public:
  void parse(const SmartMet::Spine::Value& item_def, bool allow_absent = false);

  void parse(const std::string& item_def, bool allow_absent = false);

  void reset();

  /**
   *   @brief Gets the value of parameter from request parameter map
   *
   *   In case of HTTP KVP request (POST with contant type application/x-www-form-urlencoded
   *   or simple GET request) this map is generated by
   *   SmartMet::Plugin::WFS::StoredQueryHandlerBase::parse_kvp_parameters.
   *
   *   @param req_param_map the parameter map read from HTTP request
*  @param extra_params Optional pointer to object of class
* SmartMet::Plugin::WFS::SupportsExtraHandlerParameters
*               for named parameter support
   *   @param allow_array specify whether an array is permitted in return value
   *   @return the parameter value in case of an success
   *
   *   std::vector<SmartMet::Spine::Value> may only appear in return value if allow_array == true.
   */
  boost::variant<SmartMet::Spine::Value, std::vector<SmartMet::Spine::Value> > get_value(
      const RequestParameterMap& req_param_map,
      const SupportsExtraHandlerParams* extra_params = NULL,
      bool allow_array = false) const;

  /**
   *   @brief Gets the value of parameter from request parameter map
   *
   *   In case of HTTP KVP request (POST with contant type application/x-www-form-urlencoded
   *   or simple GET request) this map is generated by
   *   SmartMet::Plugin::WFS::StoredQueryHandlerBase::parse_kvp_parameters.
   *
   *   @param result member function stores the result there when successful. Unchanged
   *            if the return value is false or an exception is thrown
   *   @param req_param_map the parameter map read from HTTP request
*  @param extra Optional pointer to object of class
* SmartMet::Plugin::WFS::SupportsExtraHandlerParameters
*               for named parameter support
   *   @param allow_array specify whether an array is permitted in return value
   *   @return indicates whether the parameter value is stored
   *
   *   std::vector<SmartMet::Spine::Value> may only appear in result value if allow_array == true.
   */
  bool get_value(
      boost::variant<SmartMet::Spine::Value, std::vector<SmartMet::Spine::Value> >& result,
      const RequestParameterMap& req_param_map,
      const SupportsExtraHandlerParams* extra = NULL,
      bool allow_array = false) const;

 private:
  bool handle_redirection(
      boost::variant<SmartMet::Spine::Value, std::vector<SmartMet::Spine::Value> >& result,
      const RequestParameterMap& req_param_map,
      const SupportsExtraHandlerParams* extra,
      const std::string& redirect_name) const;
};

}  // namespace WFS
}  // namespace Plugin
}  // namespace SmartMet
