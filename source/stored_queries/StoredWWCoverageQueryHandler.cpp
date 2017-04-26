#include "stored_queries/StoredWWCoverageQueryHandler.h"
#include <gis/Box.h>
#include <newbase/NFmiEnumConverter.h>
#include <smartmet/spine/Exception.h>

#include <boost/algorithm/string/replace.hpp>
#include <iomanip>

namespace bw = SmartMet::Plugin::WFS;

bw::StoredWWCoverageQueryHandler::StoredWWCoverageQueryHandler(
    SmartMet::Spine::Reactor* reactor,
    boost::shared_ptr<bw::StoredQueryConfig> config,
    PluginData& plugin_data,
    boost::optional<std::string> template_file_name)
    : SupportsExtraHandlerParams(config, false),
      StoredCoverageQueryHandler(reactor, config, plugin_data, template_file_name)
{
  try
  {
    itsLimitNames = config->get_mandatory_config_array<std::string>("contour_param.limitNames");

    // check number of names
    if (itsLimitNames.size() * 2 != itsLimits.size())
    {
      SmartMet::Spine::Exception exception(
          BCP, "Parameter 'contour_params.limitNames' contains wrong number of elements!");
      exception.addParameter(WFS_EXCEPTION_CODE, WFS_INVALID_PARAMETER_VALUE);
      throw exception;
    }
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", NULL);
  }
}

std::vector<bw::ContourQueryResultPtr> bw::StoredWWCoverageQueryHandler::processQuery(
    ContourQueryParameter& queryParameter) const
{
  try
  {
    std::vector<ContourQueryResultPtr> query_results;

    for (std::size_t nameIndex = 0; nameIndex < itsLimitNames.size(); nameIndex++)
    {
      std::size_t limitsIndex(nameIndex * 2);
      double lolimit = itsLimits[limitsIndex];
      double hilimit = itsLimits[limitsIndex + 1];

      std::string name = itsLimitNames[nameIndex];
      (reinterpret_cast<CoverageQueryParameter&>(queryParameter))
          .limits.push_back(SmartMet::Engine::Contour::Range(lolimit, hilimit));
    }

    // contains result for all coverages
    ContourQueryResultSet result_set = getContours(queryParameter);

    // order result set primarily by isoline value and secondarily by timestep
    unsigned int max_geoms_per_timestep = 0;
    for (auto result_item : result_set)
      if (max_geoms_per_timestep < result_item->area_geoms.size())
        max_geoms_per_timestep = result_item->area_geoms.size();

    for (std::size_t i = 0; i < max_geoms_per_timestep; i++)
    {
      for (auto result_item : result_set)
        if (i < result_item->area_geoms.size())
        {
          CoverageQueryResultPtr result(new CoverageQueryResult);
          WeatherAreaGeometry wag = result_item->area_geoms[i];
          std::size_t limitsIndex(i * 2);
          result->lolimit = itsLimits[limitsIndex];
          result->hilimit = itsLimits[limitsIndex + 1];
          result->name = itsLimitNames[i];
          result->area_geoms.push_back(wag);
          query_results.push_back(result);
        }
    }

    return query_results;
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", NULL);
  }
}

void bw::StoredWWCoverageQueryHandler::setResultHashValue(
    CTPP::CDT& resultHash, const ContourQueryResult& resultItem) const
{
  try
  {
    const CoverageQueryResult& coverageResultItem =
        reinterpret_cast<const CoverageQueryResult&>(resultItem);

    resultHash["winter_weather_type"] = coverageResultItem.name;
    resultHash["lovalue"] = coverageResultItem.lolimit;
    resultHash["hivalue"] = coverageResultItem.hilimit;
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", NULL);
  }
}

namespace
{
using namespace SmartMet::Plugin::WFS;

boost::shared_ptr<SmartMet::Plugin::WFS::StoredQueryHandlerBase>
wfs_winterweather_coverage_query_handler_create(SmartMet::Spine::Reactor* reactor,
                                                boost::shared_ptr<StoredQueryConfig> config,
                                                PluginData& plugin_data,
                                                boost::optional<std::string> template_file_name)
{
  try
  {
    StoredWWCoverageQueryHandler* qh =
        new StoredWWCoverageQueryHandler(reactor, config, plugin_data, template_file_name);
    boost::shared_ptr<SmartMet::Plugin::WFS::StoredQueryHandlerBase> result(qh);
    result->init_handler();
    return result;
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", NULL);
  }
}
}

SmartMet::Plugin::WFS::StoredQueryHandlerFactoryDef
    wfs_winterweather_coverage_query_handler_factory(
        &wfs_winterweather_coverage_query_handler_create);
