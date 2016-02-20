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

    controller.StartServer();               // start

    int i;
    while (true) {
        scanf("%d", &i);
        if (i == 0) {                       // exit
            break;
        } else if (i == 1) {
            controller.StartServer();       // resume or restart
        } else if (i == 2) {
            controller.StopServer();        // suspend
        }
    }

    return 0;
}
