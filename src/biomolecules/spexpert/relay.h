#ifndef BIOMOLECULES_SPEXPERT_RELAY_H_
#define BIOMOLECULES_SPEXPERT_RELAY_H_

#include <biomolecules/sprelay/core/k8090_defines.h>

#include "timespan.h"

namespace biomolecules {
namespace spexpert {
namespace relay {

struct Settings
{
    sprelay::core::k8090::RelayID calibration_lamp_switch_id;
    bool calibration_lamp_switch_on;
};

}  // biomolecules
}  // spexpert
}  // relay

#endif  // BIOMOLECULES_SPEXPERT_RELAY_H_

