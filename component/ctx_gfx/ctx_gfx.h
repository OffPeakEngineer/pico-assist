#pragma once

#include <FreeRTOS.h>
#include <task.h>

#include "esphome/core/component.h"

// #include "ctx_config.h"

namespace esphome {
namespace picocalc {

    class CtxGraphics: public Component {
        public:
            void setup() override;
            void dump_config() override;
            void loop() override;
        protected:
        private:
    };

}  // namespace picocalc
}  // namespace esphome