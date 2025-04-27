#include "picocalc.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"

namespace esphome
{
    namespace picocalc
    {
        static const char *const TAG = "picocalc";

        void PicoCalc::setup()
        {
            ESP_LOGCONFIG(TAG, "Setting up PicoCalc");
        }

        void PicoCalc::dump_config()
        {
            ESP_LOGCONFIG(TAG, "PicoCalc config:");
        }

        void PicoCalc::loop()
        {
            
        }

    } // namespace picocalc
} // namespace esphome