/*
 * Author: number
 * Date  : Nov 19, 2015
 */

#include "util.h"

namespace ef {

std::string JoinPath(const std::string& p1, const std::string &p2)
{
    std::string::size_type len_p1 = p1.length();
    if (p1 != "" && p1[len_p1-1] != '/') {
        return p1 + "/" + p2;
    }
    return p1 + p2;
}

} /* namespace ef */
