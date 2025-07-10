#include "ArgTransformer.h"


uint8_t ArgTransformer::parseByte(const std::string& str, int index) const {
    std::istringstream ss(str);
    int value = 0;

    if (str.rfind("0x", 0) == 0 || str.rfind("0X", 0) == 0)
        ss >> std::hex >> value;
    else
        ss >> std::dec >> value;

    if (ss.fail() || value < 0 || value > 255) {
        return 0xFF;
    }

    return static_cast<uint8_t>(value);
}

std::vector<uint8_t> ArgTransformer::parseByteList(const std::string& input) const {
    std::istringstream iss(input);
    std::string token;
    std::vector<uint8_t> bytes;

    int index = 0;
    while (iss >> token) {
        uint8_t parsed = parseByte(token, index);
        if (parsed == 0xFF && token != "0xFF" && token != "255") {
            continue; // ignore
        }
        bytes.push_back(parsed);
        index++;
    }

    return bytes;
}

uint8_t ArgTransformer::parseHexOrDec(const std::string& str) const {
    if (str.empty()) return 0;

    int base = 10;
    const char* cstr = str.c_str();

    // Check Hex format
    if (str.size() > 2 && (str[0] == '0') && (str[1] == 'x' || str[1] == 'X')) {
        base = 16;
        cstr += 2; // saute le prefixe 0x
    }

    // Verifier tous les char
    for (size_t i = 0; i < strlen(cstr); ++i) {
        char c = cstr[i];
        if (base == 10 && !isdigit(c)) return 0;
        if (base == 16 && !isxdigit(c)) return 0;
    }

    long value = strtol(str.c_str(), nullptr, base);
    if (value < 0 || value > 255) return 0;

    return static_cast<uint8_t>(value);
}


std::vector<std::string> ArgTransformer::splitArgs(const std::string& input) {
    std::vector<std::string> result;
    std::istringstream iss(input);
    std::string token;
    while (iss >> token) {
        result.push_back(token);
    }
    return result;
}

bool ArgTransformer::parseInt(const std::string& input, int& output) {
    const char* str = input.c_str();
    char* end = nullptr;
    int base = 10;

    if (input.rfind("0x", 0) == 0 || input.rfind("0X", 0) == 0) {
        base = 16;
        str += 2;
    } else if (!input.empty() && (input.back() == 'h' || input.back() == 'H')) {
        base = 16;
        std::string trimmed = input.substr(0, input.size() - 1);
        str = trimmed.c_str();
    }

    long result = strtol(str, &end, base);
    if (*end != '\0') return false; // invalid characters found

    output = static_cast<int>(result);
    return true;
}

bool ArgTransformer::isValidNumber(const std::string& input) {
    std::string s = input;
    int base = 10;
    if (s.empty()) return false;

    if (s.rfind("0x", 0) == 0 || s.rfind("0X", 0) == 0) {
        base = 16;
        s = s.substr(2);
    }

    for (char c : s) {
        if ((base == 10 && !isdigit(c)) ||
            (base == 16 && !isxdigit(c))) {
            return false;
        }
    }

    return true;
}

uint8_t ArgTransformer::toUint8(const std::string& input) {
    return static_cast<uint8_t>(std::stoi(input));
}

uint32_t ArgTransformer::toUint32(const std::string& input) {
    return static_cast<uint32_t>(std::stoul(input));
}

std::string ArgTransformer::toLower(const std::string& input) {
    std::string result = input;
    std::transform(
        result.begin(), result.end(), result.begin(),
        [](unsigned char c) { return std::tolower(c); }
    );
    return result;
}

std::string ArgTransformer::filterPrintable(const std::string& input) {
    std::string result;
    for (char c : input) {
        if (std::isprint(static_cast<unsigned char>(c)) || c == '\n' || c == '\r' || c == '\t') {
            result += c;
        }
    }
    return result;
}

std::string ArgTransformer::decodeEscapes(const std::string& input) {
    std::ostringstream out;
    for (size_t i = 0; i < input.length(); ++i) {
        if (input[i] == '\\' && i + 1 < input.length()) {
            char next = input[++i];
            switch (next) {
                case 'n': out << '\n'; break;
                case 'r': out << '\r'; break;
                case 't': out << '\t'; break;
                case '\\': out << '\\'; break;
                case 'x':
                    if (i + 2 < input.length()) {
                        std::string hex = input.substr(i + 1, 2);
                        try {
                            out << static_cast<char>(std::stoi(hex, nullptr, 16));
                        } catch (...) {
                            out << "\\x" << hex; // Invalid
                        }
                        i += 2;
                    } else {
                        out << "\\x"; // Incomplete
                    }
                    break;
                default:
                    out << '\\' << next; // Unrecognized,
                    break;
            }
        } else {
            out << input[i];
        }
    }
    return out.str();
}