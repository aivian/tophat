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

#include "Percolometer.hpp"
#include "Base.hpp"
#include "InfoBoxes/InfoBoxManager.hpp"
#include "Components.hpp"
#include "Task/ProtectedTaskManager.hpp"
#include "Form/CheckBox.hpp"
#include "Form/Frame.hpp"
#include "Screen/Timer.hpp"
#include "Look/GlobalFonts.hpp"
#include "Formatter/UserUnits.hpp"
#include "Units/Units.hpp"
#include "UIGlobals.hpp"
#include "Interface.hpp"
#include "Screen/Layout.hpp"
#include "Util/StaticString.hxx"
#include "Renderer/FinalGlideBarRenderer.hpp"
#include "Look/Look.hpp"
#include "Look/DialogLook.hpp"
#include "Screen/Canvas.hpp"
#include "Profile/Profile.hpp"
#include "ActionInterface.hpp"
#include "Language/Language.hpp"
#include "Task/Points/TaskWaypoint.hpp"
#include "Screen/SingleWindow.hpp"
#include "Form/Button.hpp"

#include "LogFile.hpp"
#include "Logger/Logger.hpp"

enum ControlIndex {
  ThermalsPlus,
  ThermalsMinus,
  MinaltPlus,
  MinaltMinus,
  PWorkPlus,
  PWorkMinus,
};


class PercolometerPanel: public BaseAccessPanel, ThreeButtonNumberLayout {
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
    virtual void OnPaint(Canvas &canvas) override;
  };

protected:
  /**
   * These 4 buttons and the mc_value frame use the layout rectangles
   * calculated in NumberButtonLayout
   */
  WndSymbolButton *thermals_plus, *thermals_minus, 
                  *minalt_plus, *minalt_minus, 
                  *pwork_plus, *pwork_minus;
  WndFrame *n_thermals_value, *min_alt_value, *P_work_value;
  PixelRect checkbox_rc;
  unsigned value_font_height;

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
  DialogLook thermals_dialog_look;

public:
  PercolometerPanel(unsigned _id)
    :BaseAccessPanel(_id), dialog_timer(*this) {}

  virtual void Prepare(ContainerWindow &parent, const PixelRect &rc) override;
  virtual void Unprepare() override;
  /* Move must discard rc and use GetMainWindow()'s ClientRect */
  virtual void Move(const PixelRect &rc) override;
  void CalculateLayout(const PixelRect &rc, unsigned value_height);

  void Refresh();

protected:
  /**
   * render the final glide periodically because
   * latency in the blackboards causes the final glide
   * renderer to not always use value updated with the
   * buttons
   */
  virtual bool OnTimer(WindowTimer &timer) override;

  /* methods from ActionListener */
  virtual void OnAction(int id) override;
};


bool
PercolometerPanel::OnTimer(WindowTimer &timer)
{
  if (timer == dialog_timer) {
    Refresh();
    return true;
  }
  return BaseAccessPanel::OnTimer(timer);
}

void
PercolometerPanel::OnAction(int action_id)
{
  if (protected_task_manager == NULL)
    return;

  const ComputerSettings &settings_computer =
    CommonInterface::GetComputerSettings();
  const GlidePolar &polar = settings_computer.polar.glide_polar_task;
  TaskBehaviour &task_behaviour = CommonInterface::SetComputerSettings().task;

  fixed n_thermals = settings_computer.percolation.n_thermals;
  fixed min_alt = settings_computer.percolation.min_alt;
  fixed P_work = settings_computer.percolation.P_work;

  switch (action_id) {
  case ThermalsPlus:
    n_thermals = std::max(n_thermals + fixed(1.0), fixed(0.0));
    ActionInterface::SetNThermals(n_thermals);
    break;
  case ThermalsMinus:
    n_thermals = std::max(n_thermals - fixed(1.0), fixed(0.0));
    ActionInterface::SetNThermals(n_thermals);
    break;
  case MinaltPlus:
    min_alt = std::max(min_alt + fixed(100.0), fixed(0.0));
    ActionInterface::SetMinAlt(min_alt);
    break;
  case MinaltMinus:
    min_alt = std::max(min_alt - fixed(100.0), fixed(0.0));
    ActionInterface::SetMinAlt(min_alt);
    break;
  case PWorkPlus:
    P_work = std::min(std::max(P_work + fixed(0.1), fixed(0.0)), fixed(1.0));
    ActionInterface::SetPwork(P_work);
    break;
  case PWorkMinus:
    P_work = std::min(std::max(P_work - fixed(0.1), fixed(0.0)), fixed(1.0));
    ActionInterface::SetPwork(P_work);
    break;
  default:
    BaseAccessPanel::OnAction(action_id);
    return;
  }
  Refresh();
}

void
PercolometerPanel::Refresh()
{
  const ComputerSettings &settings_computer =
    CommonInterface::GetComputerSettings();
  fixed n_thermals = settings_computer.percolation.n_thermals;
  StaticString<32> buffer;
  buffer.Format("%0.0f", n_thermals);
  n_thermals_value->SetCaption(buffer.c_str());

  StaticString<32> buffer_alt;
  fixed min_alt = settings_computer.percolation.min_alt;
  buffer_alt.Format("%0.0f", min_alt);
  min_alt_value->SetCaption(buffer_alt.c_str());

  StaticString<32> buffer_work;
  fixed P_work = settings_computer.percolation.P_work;
  buffer_work.Format("%0.1f", P_work);
  P_work_value->SetCaption(buffer_work.c_str());

  final_glide_chart->Invalidate();
}

void
PercolometerPanel::Move(const PixelRect &rc_unused)
{
  PixelRect rc = UIGlobals::GetMainWindow().GetClientRect();

  BaseAccessPanel::Move(rc);
  CalculateLayout(rc, value_font_height);
  final_glide_chart->Move(fg_rc);
  thermals_plus->Move(big_plus_rc);
  minalt_plus->Move(little_plus_rc);
  thermals_minus->Move(big_minus_rc);
  minalt_minus->Move(little_minus_rc);
  pwork_plus->Move(middle_plus_rc);
  pwork_minus->Move(middle_minus_rc);

  n_thermals_value->Move(big_value_rc);
  P_work_value->Move(mid_value_rc);
  min_alt_value->Move(little_value_rc);
}

void
PercolometerPanel::CalculateLayout(const PixelRect &rc, unsigned value_height)
{
  const DialogLook &dialog_look = UIGlobals::GetDialogLook();
  unsigned sub_number_height =
      dialog_look.text_font.GetHeight() + Layout::GetTextPadding();
  ThreeButtonNumberLayout::CalculateLayout(content_rc, value_height,
                                                sub_number_height);

  PixelRect content_right_rc = content_rc;
  PixelRect content_left_rc = content_rc;

  // split content area into two columns, buttons on the right, fg on left
  content_right_rc.left += Layout::Scale(50);

  ThreeButtonNumberLayout::CalculateLayout(content_right_rc, value_height,
                                                sub_number_height);
  content_left_rc.right = big_plus_rc.left - 1;
  fg_rc = content_left_rc;

  checkbox_rc.bottom = content_rc.bottom -
    (content_rc.bottom - big_minus_rc.bottom) / 4;
  checkbox_rc.top = big_minus_rc.bottom +
    (content_rc.bottom - big_minus_rc.bottom) / 4;
  checkbox_rc.left = big_minus_rc.left;
  checkbox_rc.right = little_minus_rc.right;
}

void
PercolometerPanel::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  BaseAccessPanel::Prepare(parent, rc);

  const ButtonLook &button_look = UIGlobals::GetDialogLook().button;
  const DialogLook &dialog_look = UIGlobals::GetDialogLook();
  thermals_dialog_look.Initialise(200);
  value_font_height = thermals_dialog_look.text_font.GetHeight();

  CalculateLayout(rc, value_font_height);

  WindowStyle style;
  const Look &look = UIGlobals::GetLook();
  final_glide_chart =
      new FinalGlideChart(GetClientAreaWindow(),
                          fg_rc.left, fg_rc.top,
                          (UPixelScalar)(fg_rc.right - fg_rc.left),
                          (UPixelScalar)(fg_rc.bottom - fg_rc.top),
                          style, look);

  WindowStyle button_style;
  button_style.TabStop();
  thermals_plus = new WndSymbolButton(GetClientAreaWindow(), button_look, _T("^"),
                                 big_plus_rc,
                                 button_style, *this, ThermalsPlus);

  thermals_minus = new WndSymbolButton(GetClientAreaWindow(), button_look,
                                  _T("v"), big_minus_rc,
                                  button_style, *this, ThermalsMinus);

  minalt_plus = new WndSymbolButton(GetClientAreaWindow(), button_look,
                                    _T("^"), little_plus_rc,
                                    button_style, *this, MinaltPlus);

  minalt_minus = new WndSymbolButton(GetClientAreaWindow(), button_look,
                                     _T("v"), little_minus_rc,
                                     button_style, *this, MinaltMinus);

  pwork_plus = new WndSymbolButton(GetClientAreaWindow(), button_look,
                                    _T("^"), middle_plus_rc,
                                    button_style, *this, PWorkPlus);

  pwork_minus = new WndSymbolButton(GetClientAreaWindow(), button_look,
                                     _T("v"), middle_minus_rc,
                                     button_style, *this, PWorkMinus);


  WindowStyle style_frame;
  n_thermals_value = new WndFrame(GetClientAreaWindow(), thermals_dialog_look,
                          big_value_rc, style_frame);
  n_thermals_value->SetAlignCenter();
  n_thermals_value->SetVAlignCenter();

  P_work_value = new WndFrame(GetClientAreaWindow(), thermals_dialog_look,
                          mid_value_rc, style_frame);
  P_work_value->SetAlignCenter();
  P_work_value->SetVAlignCenter();

  min_alt_value = new WndFrame(GetClientAreaWindow(), thermals_dialog_look,
                          little_value_rc, style_frame);
  min_alt_value->SetAlignCenter();
  min_alt_value->SetVAlignCenter();


  WindowStyle checkbox_style;
  checkbox_style.TabStop();

  dialog_timer.Schedule(500);
  Refresh();
}

void
PercolometerPanel::Unprepare()
{
  dialog_timer.Cancel();
  delete(final_glide_chart);
  delete(minalt_plus);
  delete(thermals_plus);
  delete(minalt_minus);
  delete(thermals_minus);
  delete(n_thermals_value);
  delete(min_alt_value);
  delete(P_work_value);
}

Widget *
LoadPercolometerPanel(unsigned id)
{
  return new PercolometerPanel(id);
}


PercolometerPanel::FinalGlideChart::FinalGlideChart(ContainerWindow &parent,
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
PercolometerPanel::FinalGlideChart::OnPaint(Canvas &canvas)
{
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
