#ifndef MY_UTILS
#define MY_UTILS

#include <string>
#include <cstring>

std::string hexStr(unsigned char *data, int len);
std::string stripColonsFromMac(const char *mac);

#endif // MY_UTILS
