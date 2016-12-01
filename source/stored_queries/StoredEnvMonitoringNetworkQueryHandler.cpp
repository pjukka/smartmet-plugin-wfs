#include "stored_queries/StoredEnvMonitoringNetworkQueryHandler.h"
#include "StoredQueryHandlerFactoryDef.h"
#include "SupportsLocationParameters.h"
#include <smartmet/spine/Exception.h>
#include <smartmet/engines/observation/DBRegistry.h>
#include <boost/icl/type_traits/to_string.hpp>
#include <memory>

namespace bw = SmartMet::Plugin::WFS;
namespace bo = SmartMet::Engine::Observation;

namespace
{
const char* P_NETWORK_ID = "networkId";
const char* P_NETWORK_NAME = "networkName";
const char* P_STATION_ID = "stationId";
const char* P_STATION_NAME = "stationName";
const char* P_MISSING_TEXT = "missingText";
}

bw::StoredEnvMonitoringNetworkQueryHandler::StoredEnvMonitoringNetworkQueryHandler(
    SmartMet::Spine::Reactor* reactor,
    boost::shared_ptr<StoredQueryConfig> config,
    PluginData& plugin_data,
    boost::optional<std::string> template_file_name)
    : bw::SupportsExtraHandlerParams(config),
      bw::StoredQueryHandlerBase(reactor, config, plugin_data, template_file_name),
      m_geoEngine(NULL),
      m_obsEngine(NULL)
{
  try
  {
    register_array_param<int64_t>(P_NETWORK_ID, *config);
    register_array_param<std::string>(P_NETWORK_NAME, *config, false);
    register_array_param<int64_t>(P_STATION_ID, *config);
    register_array_param<std::string>(P_STATION_NAME, *config, false);
    m_missingText = config->get_optional_config_param<std::string>(P_MISSING_TEXT, "NaN");
    m_debugLevel = config->get_debug_level();
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", NULL);
  }
}

bw::StoredEnvMonitoringNetworkQueryHandler::~StoredEnvMonitoringNetworkQueryHandler()
{
}

void bw::StoredEnvMonitoringNetworkQueryHandler::init_handler()
{
  try
  {
    auto* reactor = get_reactor();

    void* engine;

    // Get Geonames
    engine = reactor->getSingleton("Geonames", NULL);
    if (engine == NULL)
      throw SmartMet::Spine::Exception(BCP, "No Geonames engine available");

    m_geoEngine = reinterpret_cast<SmartMet::Engine::Geonames::Engine*>(engine);

    // Get Observation
    engine = reactor->getSingleton("Observation", NULL);
    if (engine == NULL)
      throw SmartMet::Spine::Exception(BCP, "No Observation engine available");

    m_obsEngine = reinterpret_cast<SmartMet::Engine::Observation::Interface*>(engine);

    // Set Geonames into Observation
    m_obsEngine->setGeonames(m_geoEngine);
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", NULL);
  }
}

void bw::StoredEnvMonitoringNetworkQueryHandler::query(const StoredQuery& query,
                                                       const std::string& language,
                                                       std::ostream& output) const
{
  try
  {
    const auto& params = query.get_param_map();

    try
    {
      if (m_debugLevel > 0)
        query.dump_query_info(std::cout);

      std::string lang = language;
      bw::SupportsLocationParameters::engOrFinToEnOrFi(lang);

      // Get request parameters.
      std::vector<int64_t> networkIdVector;
      params.get<int64_t>(P_NETWORK_ID, std::back_inserter(networkIdVector));

      std::vector<std::string> networkNameVector;
      params.get<std::string>(P_NETWORK_NAME, std::back_inserter(networkNameVector));

      std::vector<int64_t> stationIdVector;
      params.get<int64_t>(P_STATION_ID, std::back_inserter(stationIdVector));

      std::vector<std::string> stationNameVector;
      params.get<std::string>(P_STATION_NAME, std::back_inserter(stationNameVector));

      // Using GROUP_MEMBERS_V1 as a base configuration
      bo::MastQueryParams emnQueryParams(dbRegistryConfig("GROUP_MEMBERS_V1"));
      emnQueryParams.addField("GROUP_ID");

      // Join on NETWORKS_V1 view
      emnQueryParams.addJoinOnConfig(dbRegistryConfig("STATION_GROUPS_V2"), "GROUP_ID");
      emnQueryParams.addField("GROUP_NAME");
      emnQueryParams.addField("GROUP_DESC");

      // Join on STATIONS_V1 view
      emnQueryParams.addJoinOnConfig(dbRegistryConfig("STATIONS_V1"), "STATION_ID");
      emnQueryParams.addField("STATION_ID");
      emnQueryParams.addField("STATION_GEOMETRY.sdo_point.x", "LONGITUDE");
      emnQueryParams.addField("STATION_GEOMETRY.sdo_point.y", "LATITUDE");

      emnQueryParams.addOrderBy("GROUP_ID", "ASC");
      emnQueryParams.addOrderBy("STATION_ID", "ASC");

      int class_id = 81;
      emnQueryParams.addOperation("OR_GROUP_class_id", "CLASS_ID", "PropertyIsEqualTo", class_id);

      for (std::vector<int64_t>::const_iterator it = networkIdVector.begin();
           it != networkIdVector.end();
           ++it)
        emnQueryParams.addOperation("OR_GROUP_network", "GROUP_ID", "PropertyIsEqualTo", *it);

      for (std::vector<std::string>::const_iterator it = networkNameVector.begin();
           it != networkNameVector.end();
           ++it)
        emnQueryParams.addOperation("OR_GROUP_network", "GROUP_NAME", "PropertyIsLike", *it);

      for (std::vector<int64_t>::const_iterator it = stationIdVector.begin();
           it != stationIdVector.end();
           ++it)
        emnQueryParams.addOperation("OR_GROUP_network", "STATION_ID", "PropertyIsEqualTo", *it);

      for (std::vector<std::string>::const_iterator it = stationNameVector.begin();
           it != stationNameVector.end();
           ++it)
        emnQueryParams.addOperation("OR_GROUP_network", "STATION_NAME", "PropertyIsLike", *it);

      emnQueryParams.addOperation(
          "OR_GROUP_language_code", "LANGUAGE_CODE", "PropertyIsEqualTo", lang);

      // bo::EnvironmentalMonitoringFacilityQuery emnQuery;
      bo::MastQuery emnQuery;
      emnQuery.setQueryParams(&emnQueryParams);
      m_obsEngine->makeQuery(&emnQuery);

      CTPP::CDT hash;
      hash["responseTimestamp"] =
          boost::posix_time::to_iso_extended_string(get_plugin_data().get_time_stamp()) + "Z";
      hash["queryId"] = query.get_query_id();

      // Container with the data fetched from obsengine.
      std::shared_ptr<bo::QueryResult> resultContainer = emnQuery.getQueryResultContainer();

      int networkCount = 0;

      if (resultContainer)
      {
        bo::QueryResult::ValueVectorType::const_iterator netIdIt =
            resultContainer->begin("GROUP_ID");
        bo::QueryResult::ValueVectorType::const_iterator netIdItEnd =
            resultContainer->end("GROUP_ID");
        bo::QueryResult::ValueVectorType::const_iterator netNameIt =
            resultContainer->begin("GROUP_NAME");
        bo::QueryResult::ValueVectorType::const_iterator netDescIt =
            resultContainer->begin("GROUP_DESC");
        bo::QueryResult::ValueVectorType::const_iterator longitudeIt =
            resultContainer->begin("LONGITUDE");
        bo::QueryResult::ValueVectorType::const_iterator longitudeItEnd =
            resultContainer->end("LONGITUDE");
        bo::QueryResult::ValueVectorType::const_iterator latitudeIt =
            resultContainer->begin("LATITUDE");
        bo::QueryResult::ValueVectorType::const_iterator latitudeItEnd =
            resultContainer->end("LATITUDE");

        size_t netNameVectorSize = resultContainer->size("GROUP_NAME");
        size_t netIdVectorSize = netIdItEnd - netIdIt;
        size_t stationLongitudeVectorSize = longitudeItEnd - longitudeIt;
        size_t stationLatitudeVectorSize = latitudeItEnd - latitudeIt;

        std::string networkId;

        if (netIdVectorSize == netNameVectorSize and
            netIdVectorSize == stationLongitudeVectorSize and
            netIdVectorSize == stationLatitudeVectorSize)
        {
          SmartMet::Engine::Gis::CRSRegistry& crs_registry = get_plugin_data().get_crs_registry();

          // Networks are bounded by..
          const std::string boundenByCRS = "EPSG:4258";
          bool bbShowHeight = false;
          std::string bbProjUri;
          std::string bbProjEpochUri;
          std::string bbAxisLabels;
          crs_registry.get_attribute(boundenByCRS, "showHeight", &bbShowHeight);
          crs_registry.get_attribute(boundenByCRS, "projUri", &bbProjUri);
          crs_registry.get_attribute(boundenByCRS, "projEpochUri", &bbProjEpochUri);
          crs_registry.get_attribute(boundenByCRS, "axisLabels", &bbAxisLabels);

          // Calculating bounding box of the station coordinates.
          std::pair<double, double> longitudeMinMax =
              bo::QueryResult::minMax(longitudeIt, longitudeItEnd);
          std::pair<double, double> latitudeMinMax =
              bo::QueryResult::minMax(latitudeIt, latitudeItEnd);

          // Bounding box values
          std::string bbLowerCorner = (m_missingText + " " + m_missingText);
          std::string bbUpperCorner = (m_missingText + " " + m_missingText);
          if (longitudeMinMax.first <= longitudeMinMax.second and
              latitudeMinMax.first <= latitudeMinMax.second)
          {
            std::ostringstream lower;
            lower << std::setprecision(6) << std::fixed << latitudeMinMax.first << " "
                  << longitudeMinMax.first;
            bbLowerCorner = lower.str();
            std::ostringstream upper;
            upper << std::setprecision(6) << std::fixed << latitudeMinMax.second << " "
                  << longitudeMinMax.second;
            bbUpperCorner = upper.str();
          }

          hash["bbLowerCorner"] = bbLowerCorner;
          hash["bbUpperCorner"] = bbUpperCorner;
          hash["bbProjUri"] = bbProjUri;
          hash["bbProjEpoch_uri"] = bbProjEpochUri;
          hash["bbAxisLabels"] = bbAxisLabels;
          hash["bbProjSrsDim"] = (bbShowHeight ? 3 : 2);

          // FIXME!! Use output CRS requested
          hash["axisLabels"] = bbAxisLabels;
          hash["projUri"] = bbProjUri;
          hash["projSrsDim"] = (bbShowHeight ? 3 : 2);

          // Filling the network and station data
          int networkCounter = 0;
          for (; netIdIt != netIdItEnd; ++netIdIt, ++netNameIt, ++netDescIt)
          {
            const std::string netId = bo::QueryResult::toString(netIdIt);
            if (networkId != netId)
            {
              networkCount++;
              networkCounter++;
              hash["networks"][networkCounter - 1]["id"] = netId;
              hash["networks"][networkCounter - 1]["name"] = bo::QueryResult::toString(netNameIt);
              hash["networks"][networkCounter - 1]["description"] =
                  bo::QueryResult::toString(netDescIt);
              hash["networks"][networkCounter - 1]["inspireNamespace"] = "fi.fmi.network.id";
            }
            networkId = netId;
          }
        }
        else
        {
          std::ostringstream msg;
          msg << "warning: bw::StoredEnvMonitoringNetworkQueryHandler::query - varying size data "
                 "vectors!\n";
          std::cerr << msg.str();
        }
      }

      const std::string networksMatched = boost::to_string(networkCount);
      hash["networksMatched"] = networksMatched;
      hash["networksReturned"] = networksMatched;

      hash["fmi_apikey"] = QueryBase::FMI_APIKEY_SUBST;
      hash["fmi_apikey_prefix"] = bw::QueryBase::FMI_APIKEY_PREFIX_SUBST;
      hash["hostname"] = QueryBase::HOSTNAME_SUBST;
      hash["language"] = language;

      // Format output
      format_output(hash, output, query.get_use_debug_format());

    }  // In case of some other failures
    catch (...)
    {
      SmartMet::Spine::Exception exception(BCP, "Operation processing failed!", NULL);
      if (exception.getExceptionByParameterName(WFS_EXCEPTION_CODE) == NULL)
        exception.addParameter(WFS_EXCEPTION_CODE, WFS_OPERATION_PROCESSING_FAILED);
      exception.addParameter(WFS_LANGUAGE, language);
      throw exception;
    }
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", NULL);
  }
}

void bw::StoredEnvMonitoringNetworkQueryHandler::update_parameters(
    const RequestParameterMap& params,
    int seq_id,
    std::vector<boost::shared_ptr<RequestParameterMap> >& result) const
{
  try
  {
    std::cerr << "bw::StoredEnvMonitoringNetworkQueryHandler::update_parameters(params, seq_id, "
                 "result)\n";
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", NULL);
  }
}

const std::shared_ptr<SmartMet::Engine::Observation::DBRegistryConfig>
bw::StoredEnvMonitoringNetworkQueryHandler::dbRegistryConfig(const std::string& configName) const
{
  try
  {
    // Get database registry from Observation

    const std::shared_ptr<SmartMet::Engine::Observation::DBRegistry> dbRegistry =
        m_obsEngine->dbRegistry();
    if (not dbRegistry)
    {
      SmartMet::Spine::Exception exception(BCP, "Database registry is not available!");
      exception.addParameter(WFS_EXCEPTION_CODE, WFS_OPERATION_PROCESSING_FAILED);
      throw exception;
    }

    const std::shared_ptr<SmartMet::Engine::Observation::DBRegistryConfig> dbrConfig =
        dbRegistry->dbRegistryConfig(configName);
    if (not dbrConfig)
    {
      SmartMet::Spine::Exception exception(BCP,
                                           "Database registry configuration is not available!");
      exception.addParameter(WFS_EXCEPTION_CODE, WFS_OPERATION_PROCESSING_FAILED);
      exception.addParameter("Config name", configName);
      throw exception;
    }

    return dbrConfig;
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
wfs_stored_env_monitoring_network_handler_create(SmartMet::Spine::Reactor* reactor,
                                                 boost::shared_ptr<StoredQueryConfig> config,
                                                 PluginData& plugin_data,
                                                 boost::optional<std::string> template_file_name)
{
  try
  {
    bw::StoredEnvMonitoringNetworkQueryHandler* qh = new bw::StoredEnvMonitoringNetworkQueryHandler(
        reactor, config, plugin_data, template_file_name);
    boost::shared_ptr<SmartMet::Plugin::WFS::StoredQueryHandlerBase> instance(qh);
    instance->init_handler();
    return instance;
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", NULL);
  }
}
}

SmartMet::Plugin::WFS::StoredQueryHandlerFactoryDef
    wfs_stored_env_monitoring_network_handler_factory(
        &wfs_stored_env_monitoring_network_handler_create);

/**

@page WFS_SQ_ENV_MONITORING_NETWORK_QUERY_HANDLER Stored Query handler for querying Environmental
Monitoring Facilities

@section WFS_SQ_ENV_MONITORING_NETWORK_QUERY_HANDLER_INTRO Introduction

This handler adds support for stored queries for ....

<table border="1">
  <tr>
     <td>Implementation</td>
     <td>SmartMet::Plugin::WFS::StoredEnvMonitoringNetworkQueryHandler</td>
  </tr>
  <tr>
    <td>Constructor name (for stored query configuration)</td>
    <td>@b wfs_stored_env_monitoring_network_handler_factory</td>
  </tr>
</table>

@section WFS_SQ_ENV_MONITORING_NETWORK_QUERY_HANDLER_PARAMS Query handler built-in parameters

The following stored query handler are in use

<table border="1">

<tr>
<th>Entry name</th>
<th>Type</th>
<th>Data type</th>
<th>Description</th>
</tr>

<tr>
  <td>networkname</td>
  <td>@ref WFS_CFG_SCALAR_PARAM_TMPL</td>
  <td>string</td>
  <td>Network name.</td>
</tr>

<tr>
  <td>networkid</td>
  <td>@ref WFS_CFG_SCALAR_PARAM_TMPL</td>
  <td>int</td>
  <td>Network id.</td>
</tr>


</table>

*/
