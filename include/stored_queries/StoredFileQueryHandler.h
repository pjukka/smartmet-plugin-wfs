#pragma once

#include <list>
#include "DataSetIndex.h"
#include "stored_queries/StoredAtomQueryHandlerBase.h"

namespace SmartMet
{
namespace Plugin
{
namespace WFS
{
class StoredFileQueryHandler : public StoredAtomQueryHandlerBase,
                               virtual public SupportsExtraHandlerParams
{
 public:
  StoredFileQueryHandler(SmartMet::Spine::Reactor* reactor,
                         boost::shared_ptr<StoredQueryConfig> config,
                         PluginData& plugin_data,
                         boost::optional<std::string> template_file_name);

  virtual ~StoredFileQueryHandler();

 private:
  virtual void update_parameters(
      const RequestParameterMap& request_params,
      int seq_id,
      std::vector<boost::shared_ptr<RequestParameterMap> >& result) const;

 private:
  std::list<boost::shared_ptr<DataSetDefinition> > ds_list;
};

}  // namespace WFS
}  // namespace Plugin
}  // namespace SmartMet
