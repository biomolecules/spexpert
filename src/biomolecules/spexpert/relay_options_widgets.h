#ifndef BIOMOLECULES_SPEXPERT_RELAY_CONTROL_PANEL_H_
#define BIOMOLECULES_SPEXPERT_RELAY_CONTROL_PANEL_H_

#include <QDialog>
#include <QString>

// forward declarations
namespace biomolecules {
namespace sprelay {
namespace core {
namespace k8090 {
class K8090;
}  // namespace k8090
}  // namespace core
}  // namespace sprelay
}  // namespace biomolecules

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

}  // namespace gui
}  // namespace spexpert
}  // namespace biomolecues

#endif // BIOMOLECULES_SPEXPERT_RELAY_CONTROL_PANEL_H_
