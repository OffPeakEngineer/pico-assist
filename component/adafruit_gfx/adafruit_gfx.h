#pragma once

#include <FreeRTOS.h>
#include <task.h>

#include "esphome/core/component.h"

#include "gfxtest.h"

namespace esphome {
namespace picocalc {

class AdafruitGfx : public Component {
    public:
        void setup() override;
        void dump_config() override;
        void loop() override;
    protected:
        void delay(uint32_t ms);
};

}  // namespace picocalc
}  // namespace esphome
