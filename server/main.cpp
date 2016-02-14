#include <sys/unistd.h>

#include "controller.h"
#include "common/logger.h"

int main(int argc, char *argv[])
{
    ef::Controller controller;
    if (!controller.InitServer()) {
        LogErr("controller InitServer failed, errmsg: %s\n", controller.GetErrMsg().c_str());
        return -1;
    }

    controller.StartServer();
    sleep(1000);
    return 0;
}
