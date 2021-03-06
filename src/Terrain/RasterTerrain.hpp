/*
Copyright_License {

  Top Hat Soaring Glide Computer - http://www.tophatsoaring.org/
  Copyright (C) 2000-2016 The Top Hat Soaring Project
  A detailed list of copyright holders can be found in the file "AUTHORS".

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
}
*/

#ifndef XCSOAR_TERRAIN_RASTER_TERRAIN_HPP
#define XCSOAR_TERRAIN_RASTER_TERRAIN_HPP

#include "RasterMap.hpp"
#include "Geo/GeoPoint.hpp"
#include "Thread/Guard.hpp"
#include "Compiler.h"

#include <tchar.h>

class FileCache;
class OperationEnvironment;

/**
 * Class to manage raster terrain database, potentially with 
 * caching or demand-loading.
 * 
 */
class RasterTerrain : public Guard<RasterMap> {
public:
  friend class RoutePlannerGlue; // for route planning
  friend class ProtectedTaskManager; // for intersection
  friend class WaypointVisitorMap; // for intersection rendering

  /** invalid value for terrain */
  static constexpr short TERRAIN_INVALID = RasterBuffer::TERRAIN_INVALID;

protected:
  RasterMap map;

public:

/** 
 * Constructor.  Returns uninitialised object. 
 * 
 */
  RasterTerrain(const TCHAR *path, const TCHAR *world_file, FileCache *cache,
                OperationEnvironment &operation)
    :Guard<RasterMap>(map), map(path, world_file, cache, operation) {}

  const Serial &GetSerial() const {
    return map.GetSerial();
  }

/** 
 * Load the terrain.  Determines the file to load from profile settings.
 * 
 */
  static RasterTerrain *OpenTerrain(FileCache *cache,
                                    OperationEnvironment &operation);

  gcc_pure
  short GetTerrainHeight(const GeoPoint location) const {
    Lease lease(*this);
    return lease->GetHeight(location);
  }

  /**
   * Wrapper for GetTerrainHeight() that replaces "special" values
   * with 0.  This is used when we need some "valid" value (and not
   * some "magic" special value).  Sometimes, 0 is the best we can do.
   *
   * Use this function with care.  "0" is just a random value like any
   * other.  Don't use it for calculations where the altitude matters
   * (e.g. glide path calculations).
   */
  gcc_pure
  short GetTerrainHeightOr0(const GeoPoint location) const {
    short h = GetTerrainHeight(location);
    if (RasterBuffer::IsSpecial(h))
      /* apply fallback */
      h = 0;
    return h;
  }

  GeoPoint GetTerrainCenter() const {
    return map.GetMapCenter();
  }

};

#endif
