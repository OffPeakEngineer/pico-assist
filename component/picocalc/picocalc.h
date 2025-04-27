#pragma once

#include "esphome/core/component.h"

namespace esphome {
namespace picocalc {

class PicoCalc : public Component {
    public:
        void setup() override;
        void dump_config() override;
        void loop() override;
};

}  // namespace picocalc
}  // namespace esphome
