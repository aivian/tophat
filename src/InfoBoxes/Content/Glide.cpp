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

#include "InfoBoxes/Content/Glide.hpp"
#include "InfoBoxes/Content/Task.hpp"
#include "InfoBoxes/Panel/Panel.hpp"
#include "Engine/Util/Gradient.hpp"
#include "InfoBoxes/Data.hpp"
#include "InfoBoxes/Panel/GrAverage.hpp"
#include "InfoBoxes/Panel/Percolometer.hpp"
#include "Interface.hpp"
#include "Util/Macros.hpp"
#include "Language/Language.hpp"

#include <tchar.h>

void
UpdateInfoBoxGRInstant(InfoBoxData &data)
{
  const fixed gr = CommonInterface::Calculated().gr;

  if (!::GradientValid(gr)) {
    data.SetInvalid();
    return;
  }

  // Set Value
  data.SetValueFromGlideRatio(gr);
}

void
UpdateInfoBoxGRCruise(InfoBoxData &data)
{
  const NMEAInfo &basic = CommonInterface::Basic();
  const DerivedInfo &calculated = CommonInterface::Calculated();
  const fixed cruise_gr = calculated.cruise_gr;

  if (!::GradientValid(cruise_gr)) {
    data.SetInvalid();
    return;
  }

  // Set Value
  data.SetValueFromGlideRatio(cruise_gr);

  if (basic.location_available)
    data.SetCommentFromDistance(basic.location.DistanceS(calculated.cruise_start_location));
  else
    data.SetCommentInvalid();
}

#ifdef __clang__
/* gcc gives "redeclaration differs in 'constexpr'" */
constexpr
#endif
const InfoBoxPanel gr_average_infobox_panels[] = {
  { N_("Set average period"), LoadGrAveragePanel },
  { nullptr, nullptr }
};

const InfoBoxPanel *
InfoBoxContentGRAvg::GetDialogContent()
{
  return gr_average_infobox_panels;
}

void
InfoBoxContentGRAvg::Update(InfoBoxData &data)
{
  const fixed average_gr = CommonInterface::Calculated().average_gr;

  if (!::GradientValid(average_gr)) {
    data.SetInvalid();
    return;
  }

  // Set Value
  data.SetValueFromGlideRatio(average_gr);
}

#ifdef __clang__
/* gcc gives "redeclaration differs in 'constexpr'" */
constexpr
#endif
const InfoBoxPanel percolometer_infobox_panels[] = {
  { N_("Set percolation parameters"), LoadPercolometerPanel },
  { nullptr, nullptr }
};

const InfoBoxPanel *
InfoBoxContentPerc::GetDialogContent()
{
  return percolometer_infobox_panels;
}

void
InfoBoxContentPerc::Update(InfoBoxData &data)
{
  // Set Value
  const ComputerSettings &settings_computer =
    CommonInterface::GetComputerSettings();
  fixed n_thermals = settings_computer.percolation.n_thermals;
  fixed min_alt = settings_computer.percolation.min_alt;
  fixed P_work = settings_computer.percolation.P_work;

  const MoreData &basic = CommonInterface::Basic();
  fixed altitude = basic.nav_altitude;
  fixed ld = settings_computer.polar.glide_polar_task.GetBestLD();
  fixed glide_range = ld * (altitude - min_alt);
  fixed intensity = n_thermals * P_work / glide_range;
  fixed perc = intensity * 3.14159 / 4.51 * glide_range;

  data.SetValueFromPercolation(perc);
}

void
UpdateInfoBoxLDVario(InfoBoxData &data)
{
  const fixed ld_vario = CommonInterface::Calculated().ld_vario;

  if (!::GradientValid(ld_vario) ||
      !CommonInterface::Basic().total_energy_vario_available ||
      !CommonInterface::Basic().airspeed_available) {
    data.SetInvalid();
    return;
  }

  // Set Value
  data.SetValueFromGlideRatio(ld_vario);
}
