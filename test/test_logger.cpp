#include <stdio.h>

#include "common/logger.h"

int main(int argc, char *argv[])
{
    if (!ef::Logger::Initialize("test_logger", 1000, 5, ef::kLevelInfo))
    {
        printf("Initialize failed, errmsg: %s\n", ef::Logger::GetErrMsg().c_str());
        return -1;
    }

    for (int i = 0; i < 100; ++i) {
        LogInfo("this is a warn message: %d\n", i);
        LogDebug("this is a debug message: %d\n", i);
    }

    LogErr("this is a debug message: %13241235d\n");
    LogErr("this is a debug message: %d\n");

    return 0;
}
