#include <IRremote.hpp> // defined here to avoid conflicts
#include "InfraredService.h"

void InfraredService::configure(uint8_t tx, uint8_t rx) {
    IrSender.begin(tx);
    IrReceiver.begin(rx, ENABLE_LED_FEEDBACK);
}

void InfraredService::sendInfraredCommand(InfraredCommand command) {
    uint8_t subdevice;
    uint16_t address;
    uint16_t vendorCode;
    std::string protocolString = InfraredProtocolMapper::toString(command.getProtocol());

    switch (command.getProtocol()) {
        // Makehex seems to not handle Panasonic correctly, we use IRemote instead
        case InfraredProtocolEnum::_PANASONIC:
        case InfraredProtocolEnum::PANASONIC2: {
            subdevice = command.getSubdevice() == -1 ? 0 : command.getSubdevice(); // -1 to 0 if needed

            // Combine device and subdevice into a 16-bit integer
            uint16_t address = (static_cast<uint16_t>(command.getSubdevice()) << 8) | command.getDevice();

            // Panasonic can be used by many manufacturers in the IRDB format, we check for vendor name
            vendorCode = getKaseikyoVendorIdCode(protocolString);
            
            IrSender.sendKaseikyo(address, command.getFunction(), 0, vendorCode);
            break;
        }
        // Same for Sony20
        case InfraredProtocolEnum::SONY20: {
            subdevice = command.getSubdevice() == -1 ? 0 : command.getSubdevice();

            // Combine device and subdevice into a 16-bit integer
            uint16_t address = (static_cast<uint16_t>(command.getSubdevice()) << 8) | command.getDevice();

            IrSender.sendSony(address, command.getFunction(), 0, SIRCS_20_PROTOCOL);  
            break;
        }
        // Handle by MakeHex
        default: {
            int frequency = 38; // Default frequency, passed by reference to encodeRemoteCommand
            std::vector<float> sequence = encodeRemoteCommand(command, protocolString.c_str(), frequency);

            // Convert the sequence to uint16_t format as required by sendRaw
            uint16_t raw[sequence.size()];
            for (size_t i = 0; i < sequence.size(); ++i) {
                raw[i] = static_cast<uint16_t>(sequence[i]);
            }

            // Send the raw generated sequence with the correct frequency
            IrSender.sendRaw(raw, sequence.size(), frequency);
        }
    }
}

InfraredCommand InfraredService::receiveInfraredCommand() {
    IrReceiver.start();
    InfraredCommand command;

    if (!IrReceiver.decode()) {
        return command;
    }

    const IRData& data = IrReceiver.decodedIRData;
    IrReceiver.resume();

    // Ignore invalid
    if (data.numberOfBits == 0 || data.protocol == UNKNOWN) {
        return command;
    }

    // Mapping protocols
    switch (data.protocol) {
        case NEC:
        case NEC2:
        case ONKYO:
            command.setProtocol(_NEC);
            break;

        case DENON:
            command.setProtocol(_DENON);
            break;

        case APPLE:
            command.setProtocol(_APPLE);
            break;

        case SONY:
            command.setProtocol(SONY20);
            break;

        case JVC:
            command.setProtocol(_JVC);
            break;

        case RC5:
            command.setProtocol(_RC5);
            break;

        case RC6:
        case RC6_6_20:
            command.setProtocol(_RC6);
            break;

        case LG:
        case LG2:
        case SAMSUNG:
        case SAMSUNGLG:
        case SAMSUNG48:
            command.setProtocol(_SAMSUNG);
            break;

        case PANASONIC:
        case KASEIKYO:
        case KASEIKYO_DENON:
        case KASEIKYO_SHARP:
        case KASEIKYO_JVC:
        case KASEIKYO_MITSUBISHI:
            command.setProtocol(_PANASONIC);
            break;

        default:
            return InfraredCommand();
    }

    // Panasonic and derived
    if (data.protocol == PANASONIC || data.protocol == KASEIKYO || data.protocol == KASEIKYO_JVC ||
        data.protocol == KASEIKYO_SHARP || data.protocol == KASEIKYO_MITSUBISHI || data.protocol == KASEIKYO_DENON) {
        // PANASONIC: 16 bits = 8b subdevice + 8b device
        command.setDevice((data.address >> 8) & 0xFF);
        command.setSubdevice(data.address & 0xFF);
        
    // Samsung
    } else if (data.protocol == SAMSUNG || data.protocol == SAMSUNGLG) {
        command.setDevice(data.address & 0xFF);
        command.setSubdevice((data.address >> 8) & 0xFF);

    } else {
        // que l'adresse
        int device = data.address & 0xFF;
        int subdevice = (data.address >> 8) & 0xFF;

        // Si subdevice est 0, ignore
        command.setDevice(device);
        command.setSubdevice(subdevice == 0 ? -1 : subdevice);
    }

    // Commande principale
    command.setFunction(data.command);

    IrReceiver.stop();
    return command;
}

uint16_t InfraredService::getKaseikyoVendorIdCode(const std::string& input) {
    std::string lowerInput = input;
    std::transform(lowerInput.begin(), lowerInput.end(), lowerInput.begin(), ::tolower);

    if (lowerInput.find("panasonic") != std::string::npos) {
        return PANASONIC_VENDOR_ID_CODE;
    } else if (lowerInput.find("denon") != std::string::npos) {
        return DENON_VENDOR_ID_CODE;
    } else if (lowerInput.find("mitsubishi") != std::string::npos) {
        return MITSUBISHI_VENDOR_ID_CODE;
    } else if (lowerInput.find("sharp") != std::string::npos) {
        return SHARP_VENDOR_ID_CODE;
    } else if (lowerInput.find("jvc") != std::string::npos) {
        return JVC_VENDOR_ID_CODE;
    } else {
        return PANASONIC_VENDOR_ID_CODE; // default
    }
}