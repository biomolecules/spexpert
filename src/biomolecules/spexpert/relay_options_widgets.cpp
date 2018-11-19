#include "relay_options_widgets.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QLabel>
#include <QLayout>

#include <biomolecules/sprelay/gui/central_widget.h>

#include "appstate.h"
#include "relay.h"

namespace biomolecules {
namespace spexpert {
namespace gui {

RelayControlPanel::RelayControlPanel(biomolecules::sprelay::core::k8090::K8090* k8090, QString com_port_name,
    QWidget* parent) : QDialog{parent}
{
    setWindowTitle("Relay control panel");
    auto central_widget = new biomolecules::sprelay::gui::CentralWidget{k8090, std::move(com_port_name), this};
    auto dialog_button_box = new QDialogButtonBox(QDialogButtonBox::Close, this);
    connect(dialog_button_box, &QDialogButtonBox::rejected, this, &RelayControlPanel::reject);
    auto main_v_layout = new QVBoxLayout{this};
    main_v_layout->setContentsMargins(0, 0, 0, 0);
    main_v_layout->addWidget(central_widget);
    auto dialog_button_box_h_layout = new QHBoxLayout;
    main_v_layout->addLayout(dialog_button_box_h_layout);
    dialog_button_box_h_layout->addWidget(dialog_button_box);
    dialog_button_box_h_layout->addSpacing(18);
    main_v_layout->addSpacing(10);
    layout()->setSizeConstraint(QLayout::SetFixedSize);
}


RelaySettingsDialog::RelaySettingsDialog(AppState *appState, QWidget *parent)
    : QDialog{parent},
      calibration_id_combo_box_{new QComboBox{this}},
      calibration_action_combo_box_{new QComboBox{this}},
      dialog_button_box_{new QDialogButtonBox{QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this}},
      app_state_{appState}
{
    setupUiElements();
    connectGui();
    makeLayout();
}


void RelaySettingsDialog::accept()
{
    bool ok;
    app_state_->relaySettings()->calibration_lamp_switch_id = biomolecules::sprelay::core::k8090::from_number(
        static_cast<unsigned char>(calibration_id_combo_box_->currentData().toUInt(&ok)));
    app_state_->relaySettings()->calibration_lamp_switch_on =
        calibration_action_combo_box_->currentData().toUInt(&ok);
    QDialog::accept();
}


void RelaySettingsDialog::reject()
{
    setupComboBoxes();
    QDialog::reject();
}


void RelaySettingsDialog::setupUiElements()
{
    setWindowTitle("Relay settings");
    fillComboBoxes();
    setupComboBoxes();
}


void RelaySettingsDialog::fillComboBoxes()
{
    for (unsigned int i = 0; i < 8; ++i) {
        calibration_id_combo_box_->addItem(tr("%1").arg(i), i);
    }
    calibration_action_combo_box_->addItem(tr("ON"), true);
    calibration_action_combo_box_->addItem(tr("OFF"), false);
}


void RelaySettingsDialog::setupComboBoxes()
{
    unsigned int relay_id_repr;
    for (relay_id_repr = 0; relay_id_repr < 8; ++relay_id_repr) {
        if ((app_state_->relaySettings()->calibration_lamp_switch_id
            & static_cast<biomolecules::sprelay::core::k8090::RelayID>(1u << relay_id_repr))
            != biomolecules::sprelay::core::k8090::RelayID::None) {
            break;
        }
    }
    if (relay_id_repr > 7) {
        relay_id_repr = kInitialCalibrationId;
    }
    int relay_id_index = calibration_id_combo_box_->findData(relay_id_repr);
    if (relay_id_index < 0) {
        relay_id_index = 0;
    }
    calibration_id_combo_box_->setCurrentIndex(relay_id_index);

    int relay_action_index = calibration_action_combo_box_->findData(
        app_state_->relaySettings()->calibration_lamp_switch_on);
    if (relay_action_index < 0) {
        relay_action_index = 0;
    }
    calibration_action_combo_box_->setCurrentIndex(relay_action_index);
}


void RelaySettingsDialog::connectGui()
{
    connect(dialog_button_box_, &QDialogButtonBox::rejected, this, &RelaySettingsDialog::reject);
    connect(dialog_button_box_, &QDialogButtonBox::accepted, this, &RelaySettingsDialog::accept);
}


void RelaySettingsDialog::makeLayout()
{
    auto main_layout = new QVBoxLayout{this};
    auto grid_layout = new QGridLayout;
    main_layout->addLayout(grid_layout);
    grid_layout->addWidget(new QLabel{"Relay", this}, 0, 1, Qt::AlignHCenter);
    grid_layout->addWidget(new QLabel{"Action", this}, 0, 2, Qt::AlignHCenter);
    grid_layout->addWidget(new QLabel{"Calibration lamp switch:", this}, 1, 0);
    grid_layout->addWidget(calibration_id_combo_box_, 1, 1);
    grid_layout->addWidget(calibration_action_combo_box_, 1, 2);
    main_layout->addWidget(dialog_button_box_);
    layout()->setSizeConstraint(QLayout::SetFixedSize);
}

}  // namespace gui
}  // namespace spexpert
}  // namespace biomolecues
