#pragma once

#include "StoredQueryHandlerBase.h"
#include "SupportsExtraHandlerParams.h"
#include "SupportsBoundingBox.h"
#include "SupportsLocationParameters.h"
#include "SupportsQualityParameters.h"
#include <engines/geonames/Engine.h>
#include <engines/observation/Interface.h>
#include <engines/observation/MastQueryParams.h>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/date_time/posix_time/ptime.hpp>

namespace pt = boost::posix_time;
namespace lt = boost::local_time;
namespace ba = boost::algorithm;
namespace bl = boost::lambda;

// Stored query configuration parameter names.
const std::string P_BEGIN_TIME = "beginTime";
const std::string P_END_TIME = "endTime";
const std::string P_METEO_PARAMETERS = "meteoParameters";
const std::string P_STATION_TYPE = "stationType";
const std::string P_TIME_STEP = "timeStep";
// const std::string P_WMOS                ="wmos";
// const std::string P_LPNNS               ="lpnns";
// const std::string P_FMISIDS             ="fmisids";
const std::string P_NUM_OF_STATIONS = "numOfStations";
// const std::string P_HOURS               ="hours";
// const std::string P_WEEK_DAYS           ="weekDays";
// const std::string P_LOCALE              ="locale";
const std::string P_MISSING_TEXT = "missingText";
const std::string P_MAX_EPOCHS = "maxEpochs";
const std::string P_CRS = "crs";
const std::string P_PRODUCER_ID = "producerId";

namespace SmartMet
{
namespace Plugin
{
namespace WFS
{
class StoredMastQueryHandler : protected virtual SupportsExtraHandlerParams,
                               public StoredQueryHandlerBase,
                               protected SupportsLocationParameters,
                               protected SupportsBoundingBox,
                               protected SupportsQualityParameters
{
 public:
  StoredMastQueryHandler(SmartMet::Spine::Reactor* reactor,
                         boost::shared_ptr<StoredQueryConfig> config,
                         PluginData& plugin_data,
                         boost::optional<std::string> template_file_name);

  virtual ~StoredMastQueryHandler();

  virtual void init_handler();

  virtual void query(const StoredQuery& query,
                     const std::string& language,
                     std::ostream& output) const;

 private:
  virtual void update_parameters(
      const RequestParameterMap& request_params,
      int seq_id,
      std::vector<boost::shared_ptr<RequestParameterMap> >& result) const;

  const std::shared_ptr<SmartMet::Engine::Observation::DBRegistryConfig> dbRegistryConfig(
      const std::string& configName) const;

  double m_maxHours;
  bool m_sqRestrictions;
  bool m_supportQCParameters;

  SmartMet::Engine::Observation::Interface* m_obsEngine;
  SmartMet::Engine::Geonames::Engine* m_geoEngine;
};

}  // namespace WFS
}  // namespace Plugin
}  // namespace SmartMet
