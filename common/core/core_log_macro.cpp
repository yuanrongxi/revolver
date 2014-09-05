#include "core/core_log_macro.h"

#ifdef WIN32
SingleLogStream core_log("core", BaseLogStreamInterface::debug);
#else
BaseLogStream core_log("core", BaseLogStreamInterface::debug);
#endif

