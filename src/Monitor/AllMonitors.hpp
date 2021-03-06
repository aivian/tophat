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

#ifndef XCSOAR_ALL_MONITORS_HPP
#define XCSOAR_ALL_MONITORS_HPP

#include "Blackboard/BlackboardListener.hpp"
#include "RateLimiter.hpp"
#include "TaskStartMonitor.hpp"
#include "WindMonitor.hpp"
#include "AirspaceWarningMonitor.hpp"
#include "TaskConstraintsMonitor.hpp"
#include "TrafficWarningMonitor.hpp"

/**
 * A container that combines all monitor classes.
 */
class AllMonitors final : NullBlackboardListener, RateLimiter {
  TaskStartMonitor task_start;
  WindMonitor wind;
  AirspaceWarningMonitor airspace_warnings;
  TaskConstraintsMonitor task_constraints;
  TrafficWarningMonitor traffic_alerts;

public:
  AllMonitors();
  ~AllMonitors();

  void Reset() {
    task_start.Reset();
    wind.Reset();
    airspace_warnings.Reset();
    task_constraints.Reset();
    traffic_alerts.Reset();
  }

  void Check() {
    task_start.Check();
    wind.Check();
    airspace_warnings.Check();
    task_constraints.Check();
    traffic_alerts.Check();
  }

private:
  void OnCalculatedUpdate(const MoreData &basic,
                          const DerivedInfo &calculated) override {
    RateLimiter::Trigger();
  }

  void Run() override {
    Check();
  }
};

#endif
