#pragma once

#if __has_include("ibus.h")

#include <ibus.h>

#define IBUS_TYPE_IM_EMOJI_PICKER_ENGINE (ibus_im_emoji_picker_engine_get_type())

extern "C" {
GType ibus_im_emoji_picker_engine_get_type();
}

#endif
