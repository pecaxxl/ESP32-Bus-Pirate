#include "ThreeWireService.h"

void ThreeWireService::configure(uint8_t cs, uint8_t sk, uint8_t di, uint8_t doPin, int16_t model, bool org8) {
    auto orgMode = org8 ? EEPROM_MODE_8BIT : EEPROM_MODE_16BIT;
    eeprom_open(&eeprom, model, orgMode, cs, sk, di, doPin);
    eepromSizeBytes = getBytesByModel(orgMode, model);
    eepromOrgMode = orgMode;
}

void ThreeWireService::end() {
    gpio_reset_pin((gpio_num_t)eeprom._CS);
    gpio_reset_pin((gpio_num_t)eeprom._SK);
    gpio_reset_pin((gpio_num_t)eeprom._DI);
    gpio_reset_pin((gpio_num_t)eeprom._DO);
}

uint16_t ThreeWireService::read16(uint16_t addr) {
    return eeprom_read(&eeprom, addr);
}

uint8_t ThreeWireService::read8(uint16_t addr) {
    return static_cast<uint8_t>(eeprom_read(&eeprom, addr));
}

void ThreeWireService::write16(uint16_t addr, uint16_t value) {
    eeprom_erase(&eeprom, addr);
    eeprom_write(&eeprom, addr, value);
}

void ThreeWireService::write8(uint16_t addr, uint8_t value) {
    eeprom_erase(&eeprom, addr);
    eeprom_write(&eeprom, addr, static_cast<uint16_t>(value));
}

void ThreeWireService::writeAll(uint16_t value) {
    eeprom_write_all(&eeprom, value);
}

void ThreeWireService::erase(uint16_t addr) {
    eeprom_erase(&eeprom, addr);
}

void ThreeWireService::eraseAll() {
    eeprom_erase_all(&eeprom);
}

std::vector<uint8_t> ThreeWireService::dump8() {
    std::vector<uint8_t> result;
    for (uint16_t i = 0; i < eepromSizeBytes; ++i) {
        result.push_back(read8(i));
    }
    return result;
}

std::vector<uint16_t> ThreeWireService::dump16() {
    std::vector<uint16_t> result;
    for (uint16_t i = 0; i < eepromSizeBytes / 2; ++i) {
        result.push_back(read16(i));
    }
    return result;
}

void ThreeWireService::writeEnable() {
    eeprom_ew_enable(&eeprom);
}

void ThreeWireService::writeDisable() {
    eeprom_ew_disable(&eeprom);
}

bool ThreeWireService::isWriteEnabled() {
    return eeprom_is_ew_enabled(&eeprom);
}

std::vector<std::string> ThreeWireService::getSupportedModels() const {
    return {
        "93C46",
        "93C56",
        "93C66",
        "93C76",
        "93C86"
    };
}

int ThreeWireService::resolveModelId(const std::string& modelStr) const {
    std::string m = modelStr;
    std::transform(m.begin(), m.end(), m.begin(), ::toupper);

    if (m == "93C46") return 46;
    if (m == "93C56") return 56;
    if (m == "93C66") return 66;
    if (m == "93C76") return 76;
    if (m == "93C86") return 86;

    return -1;
}