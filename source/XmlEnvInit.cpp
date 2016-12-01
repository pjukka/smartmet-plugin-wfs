#include "XmlEnvInit.h"
#include <sstream>
#include <stdexcept>
#include <xqilla/utils/UTF8Str.hpp>
#include <xqilla/utils/XQillaPlatformUtils.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <macgyver/TypeName.h>
#include <spine/Exception.h>

namespace SmartMet
{
namespace Plugin
{
namespace WFS
{
namespace Xml
{
EnvInit::EnvInit()
{
  try
  {
    try
    {
      XQillaPlatformUtils::initialize();
    }
    catch (const xercesc::XMLException& err)
    {
      std::string str = UTF8(err.getMessage());
      SmartMet::Spine::Exception exception(BCP, "Error during Xerces-C initialization!", NULL);
      exception.addDetail(str);
      throw exception;
    }

    // Disable Xerces-C net accessor for now
    xercesc::XMLPlatformUtils::fgNetAccessor = NULL;
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", NULL);
  }
}

EnvInit::~EnvInit()
{
  try
  {
    XQillaPlatformUtils::terminate();
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", NULL);
  }
}

}  // namespace Xml
}  // namespace WFS
}  // namespace Plugin
}  // namespace SmartMet
