#include "core/core_log_macro.h"

#ifdef WIN32
SingleLogStream core_log("core", BaseLogStreamInterface::debug);
#elif defined(__ANDROID__)
extern const char* ex_log_path;
BaseLogStream core_log(ex_log_path, "core", BaseLogStreamInterface::debug);
#else
BaseLogStream core_log("core", BaseLogStreamInterface::debug);
#endif

