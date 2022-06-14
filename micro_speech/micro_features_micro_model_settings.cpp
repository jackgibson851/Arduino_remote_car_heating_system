// Modified by: Jack Gibson for EEE526 Project
// Original file from (TinyML 2019)
// Description: This file defines Labels for the 4 possible audio detections 
//              (silence, unknown, on or go)
#include "micro_features_micro_model_settings.h"

const char* kCategoryLabels[kCategoryCount] = {
    "silence",
    "unknown",
    "on",
    "go",
};
