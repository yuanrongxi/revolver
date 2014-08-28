#include "##var_prefix#_log.h"


//第一个参数是日志文件名，自动会加.log后缀
BaseLogStream ##var_prefix#_log("##var_prefix#", BaseLogStream::debug);