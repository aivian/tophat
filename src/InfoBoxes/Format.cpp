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

#include "Data.hpp"
#include "Formatter/AngleFormatter.hpp"
#include "Formatter/GlideRatioFormatter.hpp"
#include "Math/Angle.hpp"

void
InfoBoxData::SetValue(const TCHAR *format, fixed value)
{
  UnsafeFormatValue(format, (double)value);
}

void
InfoBoxData::SetValue(Angle _value, const TCHAR *suffix)
{
  assert(suffix != NULL);

  FormatBearing(value.buffer(), value.capacity(), _value, suffix);
}

void
InfoBoxData::SetValueFromBearingDifference(Angle delta)
{
  FormatAngleDelta(value.buffer(), value.capacity(), delta);
}

void
InfoBoxData::SetValueFromGlideRatio(fixed gr)
{
  FormatGlideRatio(value.buffer(), value.capacity(), gr);
}

void
InfoBoxData::SetValueFromThermalDensity(fixed lambda)
{
    FormatGlideRatio(value.buffer(), value.capacity(), lambda);
}

void
InfoBoxData::SetValueFromPercolation(fixed perc)
{
    FormatGlideRatio(value.buffer(), value.capacity(), perc);
}

void
InfoBoxData::SetComment(Angle _value, const TCHAR *suffix)
{
  assert(suffix != NULL);

  FormatBearing(comment.buffer(), comment.capacity(), _value, suffix);
}

void
InfoBoxData::SetCommentFromBearingDifference(Angle delta)
{
  FormatAngleDelta(comment.buffer(), comment.capacity(), delta);
}
