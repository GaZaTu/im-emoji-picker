#pragma once

#include <ibus.h>

#define IBUS_TYPE_RIME_ENGINE (ibus_rime_engine_get_type())

extern "C" {
GType ibus_rime_engine_get_type();
}
