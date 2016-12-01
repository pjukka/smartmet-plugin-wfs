#include <iostream>
#include "FeatureID.h"
#include "RequestParameterMap.h"

using namespace SmartMet::Plugin::WFS;

int main(int argc, char* argv[])
{
  int ret = 0;
  for (int i = 1; i < argc; i++)
  {
    const std::string id_str = argv[i];
    std::cout << "ID='" << id_str << "'" << std::endl;
    try
    {
      auto id_p = FeatureID::create_from_id(id_str);
      std::cout << "storedquery_id='" << id_p->get_stored_query_id() << std::endl;
      RequestParameterMap params(id_p->get_params());
      std::cout << "parameters='" << params << "'" << std::endl;
    }
    catch (const std::exception& err)
    {
      std::cerr << "Exception: " << err.what();
      ret = 1;
    }
  }
  return ret;
}
