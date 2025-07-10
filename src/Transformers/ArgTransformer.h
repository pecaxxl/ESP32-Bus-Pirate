#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <cstring>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <string>
#include <algorithm>

class ArgTransformer {
public:
    std::vector<uint8_t> parseByteList(const std::string& input) const;
    uint8_t parseByte(const std::string& str, int index = -1) const;
    uint8_t parseHexOrDec(const std::string& s) const ;
    bool parseInt(const std::string& input, int& output);
    std::vector<std::string> splitArgs(const std::string& input);
    bool isValidNumber(const std::string& input);
    uint8_t toUint8(const std::string& input);
    uint32_t toUint32(const std::string& input);
    std::string toLower(const std::string& input);
    std::string filterPrintable(const std::string& input);
    std::string decodeEscapes(const std::string& input);
};
