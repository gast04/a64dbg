#include <stdlib.h>
#include "Utils/Utils.h"

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

    try {
        if (str.substr(0, 2) == "0x") {
            // parse hexstring
            return std::stoull(str.c_str(), nullptr, 16);
        }
        else {
            // parse decimal string
            return std::stoull(str.c_str(), nullptr, 10);
        }
    }
    catch (void* e) {
        printf("Could not convert to u64: %s\n", str.c_str());
        return -1;
    }
}

void hexdump(uint8_t* data, uint64_t offset, uint64_t len, uint8_t format) {

    /*
        format of printing
        <addr>: 00 11 22 33 44

        TODO: improve this function
    */

    if (format != 1 && format != 4) {
        printf("[!] Invalid hexdump format, no printing\n");
        return;
    }

    uint32_t* d = (uint32_t*)data;
    if (format == 4) {
        len /= 4;
    }

    printf("%016lx: ", offset);
    for(int i = 0; i < len; ++i) {
        if (format == 1) {
            printf("%02x ", data[i]);
        }
        else if (format == 4) {
            printf("%08x ", d[i]);
        }

        if ((i%5) == 4) {
            if (i == len -1) {
                printf("\n");
            }
            else {
                printf("\n%016lx: ", offset += 20);
            }
        }
    }
    printf("\n");
}