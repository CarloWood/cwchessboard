#pragma once

#include "cwds/debug.h"
#include "debug_ostream_operators.h"

#ifdef CWDEBUG
NAMESPACE_DEBUG_CHANNELS_START

extern channel_ct event;
extern channel_ct motion_event;
extern channel_ct widget;
extern channel_ct place;
extern channel_ct countboard;
extern channel_ct clipboard;
extern channel_ct parser;
extern channel_ct clip;

NAMESPACE_DEBUG_CHANNELS_END
#endif
