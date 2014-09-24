/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2013 The XCSoar Project
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

#include "AltitudeSimulatorFullScreen.hpp"
#include "Base.hpp"
#include "InfoBoxes/InfoBoxManager.hpp"
#include "Components.hpp"
#include "Task/ProtectedTaskManager.hpp"
#include "Form/Button.hpp"
#include "Form/Frame.hpp"
#include "Look/GlobalFonts.hpp"
#include "Screen/Timer.hpp"
#include "Formatter/UserUnits.hpp"
#include "Units/Units.hpp"
#include "UIGlobals.hpp"
#include "Interface.hpp"
#include "Screen/Layout.hpp"
#include "Util/StaticString.hpp"
#include "Renderer/FinalGlideBarRenderer.hpp"
#include "Look/Look.hpp"
#include "Screen/Canvas.hpp"
#include "Profile/Profile.hpp"
#include "ActionInterface.hpp"
#include "Language/Language.hpp"
#include "Task/Points/TaskWaypoint.hpp"
#include "Blackboard/DeviceBlackboard.hpp"
#include "Formatter/UserUnits.hpp"
#include "Screen/SingleWindow.hpp"

enum ControlIndex {
  BigPlus,
  LittlePlus,
  LittleMinus,
  BigMinus,
};


class AltitudeSimulatorFullScreenPanel : public BaseAccessPanel, NumberButtonLayout {
  class FinalGlideChart: public PaintWindow
  {
  public:
    FinalGlideChart(ContainerWindow &parent,
                    PixelScalar x, PixelScalar y,
                    UPixelScalar Width, UPixelScalar Height,
                    WindowStyle style,
                    const Look&);
  protected:
    const Look& look;

    /**
     * draws the Final Glide Bar on the MC widget so the pilot can
     * adjust his MC with respect to the arrival height
     */
    FinalGlideBarRenderer *final_glide_bar_renderer;

    /**
     * draws the the Final Glide bar
     */
    virtual void OnPaint(Canvas &canvas);
  };


protected:
  /**
   * These 4 buttons and the altitude_value frame use the layout rectangles
   * calculated in NumberButtonLayout
   */
  WndButton *big_plus, *big_minus, *little_plus, *little_minus;
  WndFrame *altitude_value;

  /**
   * draws the Final Glide Bar on the MC widget so the pilot can
   * adjust his MC with respect to the arrival height
   */
  FinalGlideChart *final_glide_chart;

  /**
   * Area where canvas will draw the final glide bar
   */
  PixelRect fg_rc;

  /**
   * This timer updates the data for the final glide
   */
  WindowTimer dialog_timer;

  /**
   * Dialog look with large text font
   */
  DialogLook big_dialog_look;

  /**
   * ButtonLook with large font
   */
  ButtonLook big_button_look;

public:
  AltitudeSimulatorFullScreenPanel(unsigned id)
    :BaseAccessPanel(id), dialog_timer(*this) {}

  virtual void Prepare(ContainerWindow &parent, const PixelRect &rc);
  virtual void Unprepare();
  /* Move must discard rc and use GetMainWindow()'s ClientRect */
  virtual void Move(const PixelRect &rc) override;
  void CalculateLayout(const PixelRect &rc);
  void Refresh();

protected:
  /**
   * render the final glide periodically because
   * latency in the blackboards causes the final glide
   * renderer to not always use value updated with the
   * buttons
   */
  virtual bool OnTimer(WindowTimer &timer);

  /* methods from ActionListener */
  virtual void OnAction(int id);
};


bool
AltitudeSimulatorFullScreenPanel::OnTimer(WindowTimer &timer)
{
  if (timer == dialog_timer) {
    Refresh();
    return true;
  }
  return BaseAccessPanel::OnTimer(timer);
}

void
AltitudeSimulatorFullScreenPanel::OnAction(int action_id)
{
  const NMEAInfo &basic = CommonInterface::Basic();
  fixed altitude = basic.gps_altitude;
  fixed step = Units::ToSysAltitude(fixed(10));

  switch (action_id) {
  case BigPlus:
    altitude += step * fixed(10);
    break;
  case LittlePlus:
    altitude += step;
    break;
  case LittleMinus:
    altitude -= step;
    break;
  case BigMinus:
    altitude -= step * fixed(10);
    break;
  default:
    BaseAccessPanel::OnAction(action_id);
    return;
  }
  device_blackboard->SetAltitude(altitude);
  Refresh();
}

void
AltitudeSimulatorFullScreenPanel::Refresh()
{
  const NMEAInfo &basic = CommonInterface::Basic();

  StaticString<50> altitude;
  FormatUserAltitude(basic.gps_altitude, altitude.buffer(), true);

  altitude_value->SetCaption(altitude.get());
  final_glide_chart->Invalidate();
}

void
AltitudeSimulatorFullScreenPanel::Move(const PixelRect &rc_unused)
{
  PixelRect rc = UIGlobals::GetMainWindow().GetClientRect();

  BaseAccessPanel::Move(rc);
  CalculateLayout(rc);
  final_glide_chart->Move(fg_rc);
  big_plus->Move(big_plus_rc);
  little_plus->Move(little_plus_rc);
  big_minus->Move(big_minus_rc);
  little_minus->Move(little_minus_rc);
  altitude_value->Move(value_rc);
}

void
AltitudeSimulatorFullScreenPanel::CalculateLayout(const PixelRect &rc)
{
  NumberButtonLayout::CalculateLayout(content_rc);

  PixelRect content_right_rc = content_rc;
  PixelRect content_left_rc = content_rc;

  // split content area into two columns, buttons on the right, fg on left
  content_right_rc.left += Layout::Scale(50);

  NumberButtonLayout::CalculateLayout(content_right_rc);
  content_left_rc.right = big_plus_rc.left - 1;
  fg_rc = content_left_rc;
}

void
AltitudeSimulatorFullScreenPanel::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  BaseAccessPanel::Prepare(parent, rc);
  CalculateLayout(rc);

  WindowStyle style;
  const Look &look = UIGlobals::GetLook();

  final_glide_chart =
      new FinalGlideChart(GetClientAreaWindow(),
                          fg_rc.left, fg_rc.top,
                          (UPixelScalar)(fg_rc.right - fg_rc.left),
                          (UPixelScalar)(fg_rc.bottom - fg_rc.top),
                          style, look);
  WndForm::AddDestruct(final_glide_chart);

  big_button_look.Initialise(Fonts::map_bold);

  big_dialog_look.Initialise(Fonts::map_bold, Fonts::infobox, Fonts::map_label,
                             Fonts::infobox, Fonts::map_bold,
                             Fonts::map_bold);
  ButtonWindowStyle button_style;
  button_style.TabStop();
  button_style.multiline();
  big_plus = new WndButton(GetClientAreaWindow(), big_button_look, _T("+100"),
                           big_plus_rc,
                           button_style, *this, BigPlus);
  WndForm::AddDestruct(big_plus);

  little_plus = new WndButton(GetClientAreaWindow(), big_button_look,
                              _T("+10"), little_plus_rc,
                              button_style, *this, LittlePlus);
  WndForm::AddDestruct(little_plus);

  big_minus = new WndButton(GetClientAreaWindow(), big_button_look,
                            _T("-100"), big_minus_rc,
                            button_style, *this, BigMinus);
  WndForm::AddDestruct(big_minus);

  little_minus = new WndButton(GetClientAreaWindow(), big_button_look,
                               _T("-10"), little_minus_rc,
                               button_style, *this, LittleMinus);
  WndForm::AddDestruct(little_minus);

  WindowStyle style_frame;

  altitude_value = new WndFrame(GetClientAreaWindow(), big_dialog_look,
                                value_rc, style_frame);
  WndForm::AddDestruct(altitude_value);

  altitude_value->SetAlignCenter();
  altitude_value->SetVAlignCenter();

  dialog_timer.Schedule(500);
  Refresh();
}

void
AltitudeSimulatorFullScreenPanel::Unprepare()
{
  dialog_timer.Cancel();
}

AltitudeSimulatorFullScreenPanel::FinalGlideChart::FinalGlideChart(
    ContainerWindow &parent,
    PixelScalar X,
    PixelScalar Y,
    UPixelScalar width,
    UPixelScalar height,
    WindowStyle style,
    const Look& _look)
:look(_look)
{
  PixelRect rc (X, Y, X + width, Y + height);
  Create(parent, rc, style);
  final_glide_bar_renderer = new FinalGlideBarRenderer(look.final_glide_bar,
                                                       look.map.task);
}

void
AltitudeSimulatorFullScreenPanel::FinalGlideChart::OnPaint(Canvas &canvas)
{
  PaintWindow::OnPaint(canvas);
  canvas.SelectNullPen();
  canvas.Clear(look.dialog.background_color);
  StaticString<64> description;

  ProtectedTaskManager::Lease task_manager(*protected_task_manager);
  if (task_manager->GetMode() == TaskType::ORDERED)
    description = _("Task");
  else {
    const TaskWaypoint* wp = task_manager->GetActiveTaskPoint();
    if (wp != nullptr)
      description = wp->GetWaypoint().name.c_str();
    else
      description.clear();
  }

  final_glide_bar_renderer->Draw(canvas, GetClientRect(),
                                 CommonInterface::Calculated(),
                                 CommonInterface::GetComputerSettings().task.glide,
  CommonInterface::GetUISettings().map.final_glide_bar_mc0_enabled,
  description.c_str());
}

Widget*
LoadAltitudeSimulatorFullScreenPanel(unsigned id)
{
  const NMEAInfo &basic = CommonInterface::Basic();
  if (!basic.gps.simulator)
    return nullptr;

  return new AltitudeSimulatorFullScreenPanel(id);
}