#include "UserInputManager.h"

std::string UserInputManager::getLine() {
    std::string result;
    size_t cursorIndex = 0;

    while (true) {
        char c = terminalInput.handler();

        // Enter
        if (c == '\r' || c == '\n') break;

        // Backspace
        if ((c == '\b' || c == 127) && cursorIndex > 0) {
            cursorIndex--;
            result.erase(cursorIndex, 1);
            terminalView.print("\b \b");
            continue;
        }

        // Printable
        if (isprint(c)) {
            result.insert(cursorIndex, 1, c);
            cursorIndex++;
            terminalView.print(std::string(1, c));
        }
    }

    terminalView.println("");
    return result;
}

uint8_t UserInputManager::readValidatedUint8(const std::string& label, uint8_t def, uint8_t min, uint8_t max) {
    while (true) {
        terminalView.print(label + " [" + std::to_string(def) + "]: ");
        std::string input = getLine();
        if (input.empty()) return def;

        if (argTransformer.isValidNumber(input)) {
            uint8_t val = argTransformer.toUint8(input);
            if (val >= min && val <= max) return val;
        }

        terminalView.println("Invalid input. Must be " + std::to_string(min) + "-" + std::to_string(max));
    }
}

uint8_t UserInputManager::readValidatedUint8(const std::string& label, uint8_t defaultVal) {
    return readValidatedUint8(label, defaultVal, 0, 255);
}

uint32_t UserInputManager::readValidatedUint32(const std::string& label, uint32_t def) {
    while (true) {
        terminalView.print(label + " [" + std::to_string(def) + "]: ");
        std::string input = getLine();
        if (input.empty()) return def;

        if (argTransformer.isValidNumber(input)) {
            return argTransformer.toUint32(input);
        }

        terminalView.println("Invalid number.");
    }
}

char UserInputManager::readCharChoice(const std::string& label, char def, const std::vector<char>& allowed) {
    while (true) {
        terminalView.print(label + " [" + def + "]: ");
        std::string input = getLine();
        if (input.empty()) return def;

        char c = toupper(input[0]);
        if (std::find(allowed.begin(), allowed.end(), c) != allowed.end()) return c;

        terminalView.println("Invalid choice.");
    }
}

bool UserInputManager::readYesNo(const std::string& label, bool def) {
    while (true) {
        terminalView.print(label + " [" + (def ? "y" : "n") + "]: ");
        std::string input = getLine();
        if (input.empty()) return def;

        char c = tolower(input[0]);
        if (c == 'y') return true;
        if (c == 'n') return false;

        terminalView.println("Please answer y or n.");
    }
}

uint8_t UserInputManager::readModeNumber() {
    std::string inputDigit;

    while (true) {
        char c = terminalInput.handler();
        if (c == '\r' || c == '\n') {
            terminalView.println("");
            break;
        }

        if (std::isdigit(c)) {
            terminalView.print(std::string(1, c));
            inputDigit += c;
        }
    }

    if (inputDigit.empty()) {
        return -1;
    }

    return std::stoi(inputDigit);
}