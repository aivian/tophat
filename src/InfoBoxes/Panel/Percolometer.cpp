#include "Percolometer.hpp"
#include "Base.hpp"
#include "Profile/ProfileKeys.hpp"
#include "Profile/Profile.hpp"
#include "Form/DataField/Enum.hpp"
#include "Form/DataField/Listener.hpp"
#include "Interface.hpp"
#include "Language/Language.hpp"
#include "Form/Form.hpp"
#include "Widget/RowFormWidget.hpp"
#include "UIGlobals.hpp"
#include "Screen/Layout.hpp"
#include "Screen/SingleWindow.hpp"

enum ControlIndex {
  Spacer,
  ThermalDensity,
};

class PercolometerConfigPanel : public RowFormWidget, private DataFieldListener  { public:
  PercolometerConfigPanel()
    :RowFormWidget(UIGlobals::GetDialogLook()), form(nullptr) {}
  /**
   * the parent form
   */
  WndForm *form;

  virtual void Prepare(ContainerWindow &parent, const PixelRect &rc) override;
  virtual bool Save(bool &changed) override;
  void Show(const PixelRect &rc) override;
  virtual void Move(const PixelRect &rc) override;
  void SetForm(WndForm *_form) {
    assert(_form != nullptr);
    form = _form;
  }

protected:
  /* methods from DataFieldListener */
  virtual void OnModified(DataField &df) override;
};

void
PercolometerConfigPanel::Show(const PixelRect &rc)
{
    RowFormWidget::Show(rc);
    RowFormWidget::GetControl(Spacer).SetVisible(false);
}

void
PercolometerConfigPanel::Move(const PixelRect &rc)
{
  RowFormWidget::Move(rc);
  form->Move(UIGlobals::GetMainWindow().GetClientRect());
}

void
PercolometerConfigPanel::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  const ComputerSettings &settings_computer = CommonInterface::GetComputerSettings();

  RowFormWidget::Prepare(parent, rc);

  assert(IsDefined());

  Add(_T("Spacer"));

  static constexpr StaticEnumChoice thermal_density_list[] = {
    { lambda1km, _T("1 km") },
    { lambda2km, _T("2 km") },
    { lambda3km, _T("3 km") },
    { lambda4km, _T("4 km") },
    { lambda5km, _T("5 km") },
    { lambda6km, _T("6 km") },
    { lambda7km, _T("7 km") },
    { lambda8km, _T("8 km") },
    { lambda9km, _T("9 km") },
    { lambda10km, _T("10 km") },
    { 0 }
  };

  AddEnum(_("Thermal Density"),
          _("Average spacing of thermals km/thermal."),
          thermal_density_list, settings_computer.thermal_density, this);
}

void
PercolometerConfigPanel::OnModified(DataField &df)
{
  assert (form != nullptr);
  if (&df == &GetDataField(ThermalDensity))
      form->SetModalResult(mrOK);
}

bool
PercolometerConfigPanel::Save(bool &_changed)
{
  bool changed = false;

  ComputerSettings &settings_computer = CommonInterface::SetComputerSettings();

  changed |=
      SaveValueEnum(ThermalDensity, ProfileKeys::ThermalDensity, settings_computer.thermal_density);

  _changed |= changed;

  return true;
}

class PercolometerPanel : public BaseAccessPanel {
public:

  PercolometerPanel(unsigned _id, PercolometerConfigPanel *panel)
    :BaseAccessPanel(_id, panel) {}
  virtual void Hide();
};

void
PercolometerPanel::Hide()
{
  PercolometerConfigPanel *percolometer_config_panel =
      (PercolometerConfigPanel *)managed_widget.Get();
  assert(percolometer_config_panel);
  bool changed;
  percolometer_config_panel->Save(changed);

  BaseAccessPanel::Hide();
}

Widget *
LoadPercolometerPanel(unsigned id)
{
  PercolometerConfigPanel *inner_panel = new PercolometerConfigPanel();
  PercolometerPanel *panel = new PercolometerPanel(id, inner_panel);
  inner_panel->SetForm(panel);
  return panel;
}

