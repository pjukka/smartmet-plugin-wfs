#ifndef SMARTMET_WFS_STORED_QUERY_HANDLER_FACTORY_DEF_H__
#define SMARTMET_WFS_STORED_QUERY_HANDLER_FACTORY_DEF_H__

#include <boost/optional.hpp>
#include <boost/shared_ptr.hpp>
#include "PluginData.h"
#include "StoredQueryHandlerBase.h"
#include "StoredQueryConfig.h"

namespace SmartMet
{
namespace Plugin
{
namespace WFS
{
class PluginData;

class StoredQueryHandlerFactoryDef
{
 public:
  typedef boost::shared_ptr<StoredQueryHandlerBase> (*factory_t)(
      SmartMet::Spine::Reactor* reactor,
      boost::shared_ptr<StoredQueryConfig> config,
      PluginData& plugin_data,
      boost::optional<std::string> template_file_name);

 private:
  unsigned char signature[20];
  factory_t factory;

 public:
  StoredQueryHandlerFactoryDef(factory_t factory);

  virtual ~StoredQueryHandlerFactoryDef();

  static boost::shared_ptr<StoredQueryHandlerBase> construct(
      const std::string& symbol_name,
      SmartMet::Spine::Reactor* reactor,
      boost::shared_ptr<StoredQueryConfig> config,
      PluginData& plugin_data,
      boost::optional<std::string> template_file_name);

 private:
  static void create_signature(unsigned char* md);
};

}  // namespace WFS
}  // namespace Plugin
}  // namespace SmartMet

#endif /* SMARTMET_WFS_STORED_QUERY_HANDLER_FACTORY_DEF_H__ */
