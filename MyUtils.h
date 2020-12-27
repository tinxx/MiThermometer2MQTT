#ifndef MY_UTILS
#define MY_UTILS

#include "Arduino.h"

std::string hexStr(unsigned char *data, int len);
std::string stripColonsFromMac(const char *mac);

#endif // MY_UTILS
