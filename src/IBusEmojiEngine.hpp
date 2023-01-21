#pragma once

#if __has_include("ibus.h")

#include <ibus.h>

#define IBUS_TYPE_EMOJI_ENGINE (ibus_emoji_engine_get_type())

extern "C" {
GType ibus_emoji_engine_get_type();
}

#endif
