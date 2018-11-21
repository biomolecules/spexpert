#ifndef BIOMOLECULES_SPEXPERT_RELAY_CONTROL_PANEL_H_
#define BIOMOLECULES_SPEXPERT_RELAY_CONTROL_PANEL_H_

#include <QDialog>
#include <QString>

// forward declarations

class QComboBox;
class QDialogButtonBox;
class QDoubleSpinBox;

namespace biomolecules {
namespace sprelay {
namespace core {
namespace k8090 {
class K8090;
}  // namespace k8090
}  // namespace core
}  // namespace sprelay
}  // namespace biomolecules

class AppState;

namespace biomolecules {
namespace spexpert {
namespace gui {

class RelayControlPanel : public QDialog
{
    Q_OBJECT

public:
    RelayControlPanel(biomolecules::sprelay::core::k8090::K8090* k8090, QString com_port_name,
        QWidget* parent = nullptr);
};

class RelaySettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RelaySettingsDialog(AppState* appState, QWidget *parent = nullptr);

public slots:
    void accept() override;
    void reject() override;

private:
    const int kInitialCalibrationId = 0;

    void setupUiElements();
    void fillComboBoxes();
    void resetUiElements();
    void connectGui();
    void makeLayout();

    QComboBox* calibration_id_combo_box_;
    QComboBox* calibration_action_combo_box_;
    QDoubleSpinBox* calibration_delay_spin_box_;
    QDialogButtonBox* dialog_button_box_;

    AppState* app_state_;
};

}  // namespace gui
}  // namespace spexpert
}  // namespace biomolecues

#endif  // BIOMOLECULES_SPEXPERT_RELAY_CONTROL_PANEL_H_
