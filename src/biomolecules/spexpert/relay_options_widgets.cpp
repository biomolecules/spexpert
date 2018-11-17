#include "relay_options_widgets.h"

#include <QDialogButtonBox>
#include <QLayout>

#include <biomolecules/sprelay/gui/central_widget.h>

namespace biomolecules {
namespace spexpert {
namespace gui {

RelayControlPanel::RelayControlPanel(biomolecules::sprelay::core::k8090::K8090* k8090, QString com_port_name,
    QWidget* parent) : QDialog{parent}
{
    setWindowTitle("Relay control panel");
    auto central_widget = new biomolecules::sprelay::gui::CentralWidget{k8090, std::move(com_port_name), this};
    auto dialog_button_box = new QDialogButtonBox(QDialogButtonBox::Close);
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

}  // namespace gui
}  // namespace spexpert
}  // namespace biomolecues
