#pragma once

#include <memory>
#include <stdio.h>
#include "rime_window.hpp"

extern "C" {
extern FILE* log_file;

void log_printf(const char* format, ...);
}
