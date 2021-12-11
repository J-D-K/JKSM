#include <string>
#include <vector>
#include <curl/curl.h>

#include "curlfuncs.h"
#include "util.h"

size_t curlFuncs::writeDataString(const char *buff, size_t sz, size_t cnt, void *u)
{
    std::string *str = (std::string *)u;
    str->append(buff, 0, sz * cnt);
    return sz * cnt;
}

size_t curlFuncs::writeHeaders(const char *buff, size_t sz, size_t cnt, void *u)
{
    std::vector<std::string> *headers = (std::vector<std::string> *)u;
    headers->push_back(buff);
    return sz * cnt;
}

size_t curlFuncs::readDataFile(char *buff, size_t sz, size_t cnt, void *u)
{
    FILE *in = (FILE *)u;
    return fread(buff, sz, cnt, in);
}

size_t curlFuncs::writeDataFile(const char *buff, size_t sz, size_t cnt, void *u)
{
    FILE *f = (FILE *)u;
    return fwrite(buff, sz, cnt, f);
}

std::string curlFuncs::getHeader(const std::string& name, std::vector<std::string> *h)
{
    std::string ret = HEADER_ERROR;
    for (unsigned i = 0; i < h->size(); i++)
    {
        std::string curHeader = h->at(i);
        size_t colonPos = curHeader.find_first_of(':');
        if (curHeader.substr(0, colonPos) == name)
        {
            ret = curHeader.substr(colonPos + 2);
            break;
        }
    }
    util::stripChar('\n', ret);
    util::stripChar('\r', ret);
    return ret;
}
