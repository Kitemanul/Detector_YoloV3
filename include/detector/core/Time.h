#pragma once

#include <string>

namespace detector {

// Current local time formatted for file names: "YYYY-MM-DD_HH-MM-SS".
std::string nowForFilename();

// Current local time formatted as a SQL datetime literal: "YYYY-MM-DD HH:MM:SS".
std::string nowForSql();

} // namespace detector
