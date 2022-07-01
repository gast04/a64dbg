#include <string>
#include <vector>

std::vector<std::string> splitString(std::string const &str, const char delim);
uint64_t stringToU64(std::string const &str);
void hexdump(uint8_t* data, uint64_t offset, uint64_t len, uint8_t format = 1);
void printCmdHeader();
void printRegsMap();
