#pragma once

#ifndef WITHOUT_OBSERVATION

#include "StoredQueryHandlerBase.h"
#include "SupportsExtraHandlerParams.h"
#include <engines/geonames/Engine.h>
#include <engines/observation/Engine.h>
#include <engines/observation/MastQuery.h>

namespace SmartMet
{
namespace Plugin
{
namespace WFS
{
class StoredEnvMonitoringNetworkQueryHandler : protected virtual SupportsExtraHandlerParams,
                                               public StoredQueryHandlerBase

{
 public:
  StoredEnvMonitoringNetworkQueryHandler(SmartMet::Spine::Reactor* reactor,
                                         boost::shared_ptr<StoredQueryConfig> config,
                                         PluginData& plugin_data,
                                         boost::optional<std::string> template_file_name);
  virtual ~StoredEnvMonitoringNetworkQueryHandler();

  virtual void init_handler();

  virtual void query(const StoredQuery& query,
                     const std::string& language,
                     std::ostream& output) const;

 private:
  const std::shared_ptr<SmartMet::Engine::Observation::DBRegistryConfig> dbRegistryConfig(
      const std::string& configName) const;

  SmartMet::Engine::Geonames::Engine* m_geoEngine;
  SmartMet::Engine::Observation::Engine* m_obsEngine;

  std::string m_missingText;
  int m_debugLevel;
};

}  // namespace WFS
}  // namespace Plugin
}  // namespace SmartMet

#endif  // WITHOUT_OBSERVATION
