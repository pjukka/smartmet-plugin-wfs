#ifndef SMARTMET_WFS_STORED_QUERY_CONFIG_H__
#define SMARTMET_WFS_STORED_QUERY_CONFIG_H__

#include <map>
#include <ostream>
#include <set>
#include <string>
#include <vector>
#include <boost/optional.hpp>
#include <libconfig.h++>
#include <ctpp2/CDT.hpp>
#include <spine/ConfigBase.h>
#include "MultiLanguageString.h"
#include "StoredQueryParamDef.h"

namespace SmartMet
{
namespace Plugin
{
namespace WFS
{
class StoredQueryHandlerBase;

/**
*   @brief The stored query configuration
*
 *   An example of configuration is given below.
 *
 *   Note that part of data are for DescribeStoredQuery request only
 *   and are not used in any other way (parameters descriptions)
 *
*   @verbatim
*
*   // disable = true;
*
*   id = "fmi::wfs::test";
 *
 *   constructor_name = "fmi_wfs_handler_create";
*
*   // Configuration is searched from template directory provided in
*   // WFS plugin configuration
*   template = "wfs_test.t2c";
*
*   title = { en = "WFS test request"; fi = "WFS testikysely"; lv = "WFS testa pieprasījums"; }
*
*   abstract = { en = "More detailed description of WFS test request";
*                fi = "Yksinkohtaisempi selitys WFS testkyselystä";
*                lv = "Pilnīgāks WFS testa pieprasījuma apraksts"; }
*
*   parametrs = (
*      {
*         name = "foo";
*         title = { en = "Parameter foo"; }
*         abstract = { en = "More detailed description of parameter foo"; }
*         xmlType = "anyURI";
 *         type = "string";
*      },
*
*      {
*         name = "bar";
*         title = { en = "Parameter bar"; }
*         abstract = { en = "More detailed description of parameter  bar"; }
*         xmlType = "string";
 *         type = "double[4..12]"
*      }
*   )
*
*   returnTypeNames = [ "fooResult" ];
*   returnLanguge = "urn:ogc:def:queryLanguage:OGC-WFS::WFS_QueryExpression";
*   isPrivate = true;
 *   defaultLanguage = "en";
*
*   @endverbatim
*/
class StoredQueryConfig : public SmartMet::Spine::ConfigBase
{
 public:
  struct ParamDesc
  {
    std::string name;
    MultiLanguageStringP title;
    MultiLanguageStringP abstract;
    int min_occurs;
    int max_occurs;
    std::string xml_type;
    StoredQueryParamDef param_def;
    boost::optional<SmartMet::Spine::Value> lower_limit;
    boost::optional<SmartMet::Spine::Value> upper_limit;
    std::set<std::string> conflicts_with;

    inline ParamDesc() : used(false) {}
    inline bool isArray() const { return max_occurs > 1 or param_def.isArray(); }
    inline unsigned getMinSize() const { return min_occurs * param_def.getMinSize(); }
    inline unsigned getMaxSize() const { return max_occurs * param_def.getMaxSize(); }
    void dump(std::ostream& stream) const;

    inline void set_used() const { used = true; }
    inline bool get_used() const { return used; }
   private:
    mutable bool used;
  };

 public:
  StoredQueryConfig(const std::string& fn);
  StoredQueryConfig(boost::shared_ptr<libconfig::Config> config);
  virtual ~StoredQueryConfig();

  inline bool is_disabled() const { return disabled; }
  inline bool is_demo() const { return demo; }
  inline bool is_test() const { return test; }
  inline const std::string& get_query_id() const { return query_id; }
  inline const std::string& get_constructor_name() const { return constructor_name; }
  /**
   *   @brief Query whether CTPP2 template file name is specified
   */
  inline bool have_template_fn() const { return template_fn; }
  /**
   *   @brief Query CTPP2 template file name
   *
   *   Throws std::runtime_error if the template file name is not specified.
   */
  std::string get_template_fn() const;

  inline const std::string& get_default_language() const { return default_language; }
  std::string get_title(const std::string& language) const;

  std::string get_abstract(const std::string& language) const;

  inline const std::vector<std::string>& get_return_type_names() const { return return_type_names; }
  inline const std::map<std::string, ParamDesc>& get_param_descriptions() const
  {
    return param_map;
  }

  inline std::size_t get_num_param_desc() const { return param_names.size(); }
  inline const ParamDesc& get_param_desc(int ind) const
  {
    return param_map.at(param_names.at(ind));
  }

  inline const std::string& get_param_name(int ind) const
  {
    return param_map.at(param_names.at(ind)).name;
  }

  std::string get_param_title(int ind, const std::string& language) const;

  std::string get_param_abstract(int ind, const std::string& language) const;

  inline const std::string& get_param_xml_type(int ind) const
  {
    return param_map.at(param_names.at(ind)).xml_type;
  }

  inline int get_debug_level() const { return debug_level; }
  inline int get_expires_seconds() const { return expires_seconds; }
  void dump_params(std::ostream& stream) const;

  void warn_about_unused_params(const StoredQueryHandlerBase* handler = NULL);

 private:
  void parse_config();

 private:
  std::string query_id;
  int expires_seconds;  ///< For the expires entity-header field. After that the response is
                        /// considered stale.
                        /**
*  @brief The name of factory method procedure for creating request handler object
*/
  std::string constructor_name;

  boost::optional<std::string> template_fn;
  std::string default_language;
  MultiLanguageStringP title;
  MultiLanguageStringP abstract;
  std::vector<std::string> return_type_names;
  std::map<std::string, ParamDesc> param_map;
  std::vector<std::string> param_names;
  bool disabled;
  bool demo;
  bool test;
  int debug_level;
};

}  // namespace WFS
}  // namespace Plugin
}  // namespace SmartMet

#endif /* SMARTMET_WFS_STORED_QUERY_CONFIG_H__ */