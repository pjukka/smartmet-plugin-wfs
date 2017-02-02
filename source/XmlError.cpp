#include "XmlError.h"
#include <boost/format.hpp>
#include <macgyver/TypeName.h>
#include <spine/Exception.h>

namespace bwx = SmartMet::Plugin::WFS::Xml;

bwx::XmlError::XmlError(const std::string& text, error_level_t error_level = bwx::XmlError::ERROR)
    : std::runtime_error(text), error_level(error_level)
{
}

bwx::XmlError::~XmlError() throw()
{
}

void bwx::XmlError::add_messages(const std::list<std::string>& messages)
{
  try
  {
    this->messages = messages;
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", NULL);
  }
}

const char* bwx::XmlError::error_level_name(enum error_level_t error_level)
{
  try
  {
    using boost::str;
    using boost::format;
    switch (error_level)
    {
      case ERROR:
        return "ERROR";
      case FATAL_ERROR:
        return "FATAL_ERROR";
      default:
        throw SmartMet::Spine::Exception(
            BCP, str(format("XmlError: unknown error level %2%") % (int)error_level));
    }
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", NULL);
  }
}