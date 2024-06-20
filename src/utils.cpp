#include <string>
#include "utils.h"

std::string getFullPath(const std::string& filename) {
    return std::string(CURRENT_WORKING_DIR) + "/" + filename;
}