/*
 * Author: number
 * Date  : Nov 3, 2015
 */

#ifndef __COMMON_SHIFT_WRITER_H_H___
#define __COMMON_SHIFT_WRITER_H_H___

#include <stdint.h>

#include <string>

namespace ef {

/** * 支持按照文件大小对文件及进行切分,
 */
class ShiftWriter {
public:
    ShiftWriter();
    virtual ~ShiftWriter();

    bool Initialize(const char * path, uint32_t max_file_size,
                   uint32_t max_file_number, const char *suffix);

    int Write(void * buf, uint32_t len);
    const std::string & GetErrMsg() const;

private:
    bool Shift();

private:
    int _fd;
    std::string _path;
    std::string _suffix;
    uint32_t _max_file_size;
    uint32_t _max_file_num;
    uint32_t _cur_file_size;

    std::string _errmsg;
};

} /* namespace ef */

#endif /* __COMMON_SHIFT_WRITER_H__ */
