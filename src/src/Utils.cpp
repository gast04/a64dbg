#include <stdlib.h>
#include "Utils.h"

std::vector<std::string> splitString(std::string const &str, const char delim)
{
    size_t start;
    size_t end = 0;
    std::vector<std::string> out;

    while ((start = str.find_first_not_of(delim, end)) != std::string::npos)
    {
        end = str.find(delim, start);
        out.push_back(str.substr(start, end - start));
    }

    return out;
}

uint64_t stringToU64(std::string const &str) {

    if (str.substr(0, 2) == "0x") {
        // parse hexstring
        return std::stoull(str.c_str(), nullptr, 16);
    }
    else {
        // parse decimal string
        return std::stoull(str.c_str(), nullptr, 10);
    }
}