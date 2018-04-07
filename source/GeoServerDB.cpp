#include "GeoServerDB.h"
#include <spine/Exception.h>

namespace bw = SmartMet::Plugin::WFS;

bw::GeoServerDB::GeoServerDB(const std::string& conn_str, std::size_t keep_conn)
    : conn_str(conn_str), conn_pool(boost::bind(&bw::GeoServerDB::create_new_conn, this), keep_conn)
{
  try
  {
    // Ensure that one can connect
    conn_pool.get();
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", NULL);
  }
}

bw::GeoServerDB::~GeoServerDB() {}

boost::shared_ptr<pqxx::connection> bw::GeoServerDB::get_conn()
{
  try
  {
    return conn_pool.get();
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", NULL);
  }
}

void bw::GeoServerDB::update()
{
  try
  {
    conn_pool.update();
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", NULL);
  }
}

boost::shared_ptr<pqxx::connection> bw::GeoServerDB::create_new_conn()
{
  try
  {
    return boost::shared_ptr<pqxx::connection>(new pqxx::connection(conn_str));
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", NULL);
  }
}
