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

#ifndef XCSOAR_SCREEN_TEXT_IN_BOX_HPP
#define XCSOAR_SCREEN_TEXT_IN_BOX_HPP

#include "Screen/Point.hpp"
#include "LabelShape.hpp"

#include <tchar.h>

class Canvas;
class LabelBlock;

struct TextInBoxMode {
  enum Alignment : uint8_t {
    LEFT,
    CENTER,
    RIGHT,
  };

  enum VerticalPosition : uint8_t {
    ABOVE,
    CENTERED,
    BELOW,
  };

  LabelShape shape;
  Alignment align;
  VerticalPosition vertical_position;
  bool move_in_view;
  /**
   * force opaque background
   */
  bool opaque;

  constexpr TextInBoxMode()
    :shape(LabelShape::SIMPLE), align(Alignment::LEFT),
     vertical_position(VerticalPosition::BELOW),
     move_in_view(false), opaque(false) {}
};

/**
 * returns the padding to the left of the text
 * left of the X parameter passed to TextInBox)
 */
unsigned TextInBoxGetLeftPadding();

/**
 * Calculates and returns box perimiter size and text size
 * @param text text to be printed
 * @param mode
 * @param tssize set to size of text
 * @return outer perimeter size of label box
 */
PixelSize
TextInBoxGetSize(const Canvas &canvas, const TCHAR *text, TextInBoxMode mode, PixelSize &tsize);

/**
 * @param label_block_skip_check.  If true, and label_block no null,
 *    will add to the label_block without checking for overlap
 */
bool
TextInBox(Canvas &canvas, const TCHAR *value,
          PixelScalar x, PixelScalar y,
          TextInBoxMode mode, const PixelRect &map_rc,
          LabelBlock *label_block=nullptr,
          bool label_block_skip_check = false);

bool
TextInBox(Canvas &canvas, const TCHAR *value, PixelScalar x, PixelScalar y,
          TextInBoxMode mode,
          UPixelScalar screen_width, UPixelScalar screen_height,
          LabelBlock *label_block=nullptr,
          bool label_block_skip_check = false);

#endif
