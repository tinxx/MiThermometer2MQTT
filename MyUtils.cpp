#include "MyUtils.h"


// Thanks to Mat at StackExchange for hex string conversion.
// https://codereview.stackexchange.com/questions/78535/converting-array-of-bytes-to-the-hex-string-representation
constexpr char hexmap[] = {'0', '1', '2', '3', '4', '5', '6', '7',
                           '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
std::string hexStr(unsigned char *data, int len) {
  std::string s(len * 2, ' ');
  for (int i = 0; i < len; ++i) {
    s[2 * i]     = hexmap[(data[i] & 0xF0) >> 4];
    s[2 * i + 1] = hexmap[data[i] & 0x0F];
  }
  return s;
}

std::string stripColonsFromMac(const char *input) {
  char output[strlen("001122334455") + 1];
  sprintf(output,
          "%.2s%.2s%.2s%.2s%.2s%.2s",
          input,
          input + 3,
          input + 6,
          input + 9,
          input + 12,
          input + 15);
  return std::string(output);
}
