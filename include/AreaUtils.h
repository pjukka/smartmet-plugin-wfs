#pragma once

#include <vector>
#include <gdal/ogr_geometry.h>
#include <newbase/NFmiPoint.h>
#include <newbase/NFmiRect.h>
#include <newbase/NFmiArea.h>

namespace SmartMet
{
namespace Plugin
{
namespace WFS
{
std::vector<NFmiPoint> bbox_exclude_point(const NFmiPoint& p1,
                                          const NFmiPoint& p2,
                                          const NFmiPoint& xy,
                                          double w_coeff = 0.000001);

void get_latlon_boundary(const NFmiArea* area,
                         OGRPolygon* result,
                         int NP = 30,
                         double w_coeff = 0.000001);
}  // namespace WFS
}  // namespace Plugin
}  // namespace SmartMet
