#include "RequestBase.h"
#include "QueryBase.h"
#include "WfsConst.h"
#include "WfsException.h"
#include "XmlUtils.h"
#include <boost/algorithm/string.hpp>
#include <macgyver/StringConversion.h>
#include <macgyver/TypeName.h>
#include <spine/Exception.h>
#include <sstream>

namespace bw = SmartMet::Plugin::WFS;
namespace bwx = SmartMet::Plugin::WFS::Xml;

namespace ba = boost::algorithm;

bw::RequestBase::RequestBase(const std::string& language)
    : language(language), soap_request(false), status(SmartMet::Spine::HTTP::not_a_status)
{
}

bw::RequestBase::~RequestBase() {}

void bw::RequestBase::set_is_soap_request(bool value)
{
  soap_request = value;
}

std::string bw::RequestBase::extract_request_name(const xercesc::DOMDocument& document)
{
  try
  {
    const char* FN = "SmartMet::Plugin::WFS::Xml::RequestBase::extractRequestName()";

    const xercesc::DOMElement* root = get_xml_root(document);
    auto name_info = bwx::get_name_info(root);

    if (name_info.second == SOAP_NAMESPACE_URI)
    {
      std::ostringstream msg;
      msg << FN << ": HTTP/SOAP requests are not yet supported";
      throw SmartMet::Spine::Exception(BCP, msg.str());
    }
    else if (name_info.second == WFS_NAMESPACE_URI)
    {
      return name_info.first;
    }
    else
    {
      if (name_info.second == "")
      {
        std::ostringstream msg;
        msg << "SmartMet::Plugin::WFS::Xml::RequestBase::extractRequestName():"
            << " no namespace provided but " << WFS_NAMESPACE_URI << " expected";
        throw SmartMet::Spine::Exception(BCP, msg.str());
      }
      else
      {
        std::ostringstream msg;
        msg << "SmartMet::Plugin::WFS::Xml::RequestBase::extractRequestName():"
            << " unexpected XML namespace " << name_info.second << " when " << WFS_NAMESPACE_URI
            << " expected";
        throw SmartMet::Spine::Exception(BCP, msg.str());
      }
    }

    return name_info.first;
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", NULL);
  }
}

std::string bw::RequestBase::extract_request_name(
    const SmartMet::Spine::HTTP::Request& http_request)
{
  try
  {
    auto req_name = http_request.getParameter("request");
    if (not req_name)
    {
      SmartMet::Spine::Exception exception(BCP, "The 'request' parameter is missing!");
      exception.addParameter(WFS_EXCEPTION_CODE, WFS_OPERATION_PARSING_FAILED);
      throw exception;
    }

    return *req_name;
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", NULL);
  }
}

bool bw::RequestBase::may_validate_xml() const
{
  return true;
}

void bw::RequestBase::set_fmi_apikey(const std::string& fmi_apikey)
{
  this->fmi_apikey = fmi_apikey;
}

void bw::RequestBase::set_fmi_apikey_prefix(const std::string& fmi_apikey_prefix)
{
  this->fmi_apikey_prefix = fmi_apikey_prefix;
}

void bw::RequestBase::set_hostname(const std::string& hostname)
{
  this->hostname = hostname;
}

void bw::RequestBase::set_protocol(const std::string& protocol)
{
  this->protocol = protocol;
}

void bw::RequestBase::substitute_all(const std::string& src, std::ostream& output) const
{
  try
  {
    std::ostringstream tmp1;
    substitute_fmi_apikey(src, tmp1);
    std::ostringstream tmp2;
    substitute_hostname(tmp1.str(), tmp2);
    substitute_protocol(tmp2.str(), output);
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", NULL);
  }
}

void bw::RequestBase::substitute_fmi_apikey(const std::string& src, std::ostream& output) const
{
  try
  {
    std::ostringstream tmp_out;
    ba::replace_all_copy(std::ostreambuf_iterator<char>(tmp_out),
                         src,
                         std::string(bw::QueryBase::FMI_APIKEY_PREFIX_SUBST),
                         fmi_apikey_prefix ? *fmi_apikey_prefix : std::string(""));

    ba::replace_all_copy(std::ostreambuf_iterator<char>(output),
                         tmp_out.str(),
                         std::string(bw::QueryBase::FMI_APIKEY_SUBST),
                         fmi_apikey ? *fmi_apikey : std::string(""));
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", NULL);
  }
}

void bw::RequestBase::substitute_hostname(const std::string& src, std::ostream& output) const
{
  try
  {
    ba::replace_all_copy(std::ostreambuf_iterator<char>(output),
                         src,
                         std::string(bw::QueryBase::HOSTNAME_SUBST),
                         hostname ? *hostname : std::string("localhost"));
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", NULL);
  }
}

void bw::RequestBase::substitute_protocol(const std::string& src, std::ostream& output) const
{
  try
  {
    ba::replace_all_copy(std::ostreambuf_iterator<char>(output),
                         src,
                         std::string(bw::QueryBase::PROTOCOL_SUBST),
                         (protocol ? *protocol : "http") + "://");
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", NULL);
  }
}

void bw::RequestBase::set_http_status(SmartMet::Spine::HTTP::Status status) const
{
  this->status = status;
}

void bw::RequestBase::check_request_name(const SmartMet::Spine::HTTP::Request& http_request,
                                         const std::string& name)
{
  try
  {
    const std::string actual_name = extract_request_name(http_request);
    if (Fmi::ascii_tolower_copy(actual_name) != Fmi::ascii_tolower_copy(name))
    {
      std::ostringstream msg;
      msg << "SmartMet::Plugin::WFS::RequestBase::check_request_name(): [INTERNAL ERROR]:"
          << " conflicting reqest name '" << actual_name << "' in HTTP request ('" << name
          << "' expected)";
      throw SmartMet::Spine::Exception(BCP, msg.str());
    }
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", NULL);
  }
}

void bw::RequestBase::check_request_name(const xercesc::DOMDocument& document,
                                         const std::string& name)
{
  try
  {
    const std::string actual_name = extract_request_name(document);
    if (Fmi::ascii_tolower_copy(actual_name) != Fmi::ascii_tolower_copy(name))
    {
      std::ostringstream msg;
      msg << "SmartMet::Plugin::WFS::RequestBase::check_request_name(): [INTERNAL ERROR]:"
          << " conflicting reqest name '" << actual_name << "' in XML document ('" << name
          << "' expected)";
      throw SmartMet::Spine::Exception(BCP, msg.str());
    }
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", NULL);
  }
}

void bw::RequestBase::check_mandatory_attributes(const xercesc::DOMDocument& document)
{
  try
  {
    struct
    {
      const char* name;
      const char* value;
    } mandatory_attr_def[] = {{"version", "2.0.0"}, {"service", "WFS"}};

    int num_mandatory_attr_def = sizeof(mandatory_attr_def) / sizeof(*mandatory_attr_def);
    const xercesc::DOMElement* root = get_xml_root(document);
    for (int i = 0; i < num_mandatory_attr_def; i++)
    {
      const auto& item = mandatory_attr_def[i];
      bwx::verify_mandatory_attr_value(*root, WFS_NAMESPACE_URI, item.name, item.value);
    }
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", NULL);
  }
}

void bw::RequestBase::check_output_format_attribute(const std::string& value)
{
  try
  {
    namespace ba = boost::algorithm;

    std::vector<std::string> w;
    ba::split(w, value, ba::is_any_of(";"));
    if ((w.size() != 2) or
        (Fmi::ascii_tolower_copy(ba::trim_copy(w[0])) != "application/gml+xml") or
        (Fmi::ascii_tolower_copy(ba::trim_copy(w[1])) != "version=3.2"))
    {
      std::ostringstream msg;
      msg << "Unsupported output format '" << value
          << "' ('application/gml+xml; version=3.2' expected)";
      SmartMet::Spine::Exception exception(BCP, msg.str());
      exception.addParameter(WFS_EXCEPTION_CODE, WFS_OPERATION_PARSING_FAILED);
      throw exception;
    }
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", NULL);
  }
}

const xercesc::DOMElement* bw::RequestBase::get_xml_root(const xercesc::DOMDocument& document)
{
  try
  {
    xercesc::DOMElement* root = document.getDocumentElement();
    if (root == NULL)
    {
      SmartMet::Spine::Exception exception(BCP, "The XML root element is missing!");
      exception.addParameter(WFS_EXCEPTION_CODE, WFS_OPERATION_PARSING_FAILED);
      throw exception;
    }
    return root;
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", NULL);
  }
}

void bw::RequestBase::check_wfs_version(const SmartMet::Spine::HTTP::Request& request)
{
  try
  {
    auto version = request.getParameter("version");
    if (version and *version != "2.0.0")
    {
      SmartMet::Spine::Exception exception(BCP, "Unsupported WFS version!");
      exception.addDetail("Only version '2.0.0' is supported.");
      exception.addParameter(WFS_EXCEPTION_CODE, WFS_INVALID_PARAMETER_VALUE);
      exception.addParameter("Requested version", *version);
      throw exception;
    }
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", NULL);
  }
}
