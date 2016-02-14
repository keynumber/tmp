/*
 * Author: number
 * Date  : Nov 3, 2015
 */

#include "shift_writer.h"

#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <unistd.h>

#include "wrapper.h"

namespace ef {

ShiftWriter::ShiftWriter()
    : _fd(STDOUT_FILENO)
    , _max_file_size(20*1024*1024)
    , _max_file_num(10)
    , _cur_file_size(0)
{
}

ShiftWriter::~ShiftWriter()
{
    if (_fd >= 0) {
        safe_close(_fd);
        _fd = -1;
    }
}

bool ShiftWriter::Initialize(const char * path,
                            uint32_t max_file_size,
                            uint32_t max_file_number,
                            const char *suffix)
{
    _path = path;
    _max_file_num = max_file_number;
    _max_file_size = max_file_size;
    _suffix = suffix;

    // to test whether the path is valid
    char file[256];
    snprintf(file, 256, "%s%d%s", _path.c_str(), 0, _suffix.c_str());
    _fd = open(file,
            O_CREAT | O_APPEND | O_WRONLY | O_NOFOLLOW | O_NOCTTY,
            S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
    if (_fd < 0) {
        _errmsg = safe_strerror(errno);
        return false;
    }

    return true;
}

int ShiftWriter::Write(void* buf, uint32_t len)
{
    int ret = safe_write(_fd, buf, len);
    if (ret < 0) {
        _errmsg = safe_strerror(errno);
    }

    _cur_file_size += len;
    // shift最好放在后边，这样如果刚好写完一个文件，就会生成另一个文件
    // 否则可能会看到没有log主文件
    if (_cur_file_size > _max_file_size && !Shift()) {
        return -1;
    }

    return ret;
}

bool ShiftWriter::Shift()
{
    if (_fd == STDOUT_FILENO) {
        return true;;
    }

    if (_fd >= 0) {
        safe_close(_fd);
        _fd = -1;
    }

    {
        char new_file[256];
        char old_file[256];
        for (int i = (int)_max_file_num - 2; i >= 0; --i) {
            snprintf(old_file, 256, "%s%d%s", _path.c_str(), i, _suffix.c_str());
            if (access(old_file, F_OK) != 0) {
                continue;
            }

            snprintf(new_file, 256, "%s%d%s", _path.c_str(), i+1, _suffix.c_str());
            rename(old_file, new_file);
        }
    }

    char file[256];
    snprintf(file, 256, "%s%d%s", _path.c_str(), 0, _suffix.c_str());
    _fd = open(file,
            O_CREAT | O_APPEND | O_WRONLY | O_NOFOLLOW | O_NOCTTY,
            S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
    if (_fd < 0) {
        _errmsg = safe_strerror(errno);
        return false;
    }

    _cur_file_size = 0;
    return true;
}

const std::string & ShiftWriter::GetErrMsg() const
{
    return _errmsg;
}

} /* namespace ef */
