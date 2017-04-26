#include "WfsConvenience.h"
#include "WfsException.h"
#include <boost/algorithm/string.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/foreach.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/spirit/include/qi.hpp>
#include <macgyver/TimeParser.h>
#include <spine/Exception.h>
#include <cctype>
#include <sstream>

namespace SmartMet
{
namespace Plugin
{
namespace WFS
{
std::string xml_escape(const std::string& src)
{
  try
  {
    std::ostringstream output;

    BOOST_FOREACH (char c, src)
    {
      switch (c)
      {
        case '<':
          output << "&lt;";
          break;
        case '>':
          output << "&gt;";
          break;
        case '&':
          output << "&amp;";
          break;
        case '\'':
          output << "&apos;";
          break;
        case '"':
          output << "&quot;";
          break;
        default:
          output << c;
          break;
      }
    }

    return output.str();
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", NULL);
  }
}

std::string get_mandatory_header(const SmartMet::Spine::HTTP::Request& request,
                                 const std::string& name)
{
  try
  {
    auto value = request.getHeader(name);
    if (not value)
    {
      SmartMet::Spine::Exception exception(
          BCP, "Missing the mandatory HTTP request header '" + name + "'!");
      exception.addParameter(WFS_EXCEPTION_CODE, WFS_OPERATION_PARSING_FAILED);
      throw exception;
    }

    return *value;
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", NULL);
  }
}

void check_time_interval(const boost::posix_time::ptime& start,
                         const boost::posix_time::ptime& end,
                         double max_hours)
{
  try
  {
    namespace pt = boost::posix_time;

    if (start > end)
    {
      SmartMet::Spine::Exception exception(BCP, "Invalid time interval!");
      exception.addDetail("The start time is later than the end time.");
      exception.addParameter(WFS_EXCEPTION_CODE, WFS_OPERATION_PARSING_FAILED);
      exception.addParameter("Start time", pt::to_simple_string(start));
      exception.addParameter("End time", pt::to_simple_string(end));
      throw exception;
    }

    pt::time_duration interval_length = end - start;
    int mi_hours = static_cast<int>(std::floor(max_hours));
    int mi_sec = static_cast<int>(std::floor(3600.0 * (max_hours - mi_hours)));
    if (interval_length > pt::hours(mi_hours) + pt::seconds(mi_sec))
    {
      SmartMet::Spine::Exception exception(BCP, "Too long time interval requested!");
      exception.addDetail("No more than " + std::to_string(max_hours) + " hours allowed.");
      exception.addParameter(WFS_EXCEPTION_CODE, WFS_OPERATION_PROCESSING_FAILED);
      exception.addParameter("Start time", pt::to_simple_string(start));
      exception.addParameter("End time", pt::to_simple_string(end));
      throw exception;
    }
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", NULL);
  }
}

void assert_unreachable(const char* file, int line)
{
  std::ostringstream msg;
  msg << "INTERNAL ERROR in " << file << " at line " << line << ": not supposed to get here";
  throw SmartMet::Spine::Exception(BCP, msg.str());
}

std::string as_string(const std::vector<SmartMet::Spine::Value>& src)
{
  try
  {
    std::ostringstream ost;
    ost << "(ValueArray ";
    BOOST_FOREACH (const auto& item, src)
    {
      ost << item;
    }
    ost << ")";
    return ost.str();
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", NULL);
  }
}

std::string as_string(
    const boost::variant<SmartMet::Spine::Value, std::vector<SmartMet::Spine::Value> >& src)
{
  try
  {
    std::ostringstream ost;
    if (src.which() == 0)
    {
      ost << boost::get<SmartMet::Spine::Value>(src);
    }
    else
    {
      ost << as_string(boost::get<std::vector<SmartMet::Spine::Value> >(src));
    }
    return ost.str();
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", NULL);
  }
}

std::string remove_trailing_0(const std::string& src)
{
  try
  {
    std::string tmp = boost::algorithm::trim_copy(src);
    if ((tmp.find_first_not_of("+-.0123456789") == std::string::npos) and
        (tmp.find('.') != std::string::npos))
    {
      const char* c1 = tmp.c_str();
      const char* c2 = c1 + tmp.length();
      while (*(c2 - 1) == '0' and std::isdigit(*(c2 - 2)))
        c2--;
      return std::string(c1, c2);
    }
    else
    {
      return src;
    }
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", NULL);
  }
}

}  // namespace WFS
}  // namespace Plugin
}  // namespace SmartMet
