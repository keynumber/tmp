#include <string.h>

#include "common/shift_writer.h"

int main(int argc, char *argv[])
{
    ef::ShiftWriter writer;
    if (!writer.Initialize("./test_shift_write", 500, 5, ".log")) {
        printf("writer initialize failed, errmsg: %s\n", writer.GetErrMsg().c_str());
        return -1;
    }

    char buf[1024];
    for (int i=0; i<100; i++) {
        snprintf(buf, 1024, "this is just a test for ShiftWriter, line number %d\n", i);
        writer.Write(buf, strlen(buf));
    }
    return 0;
}
