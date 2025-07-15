#include "Services/SpiService.h"

#include <ESP32SPISlave.h>


void SpiService::configure(uint8_t mosi, uint8_t miso, uint8_t sclk, uint8_t cs, uint32_t frequency) {
    csPin = cs;
    spiFrequency = frequency;
    SPI.begin(sclk, miso, mosi, cs);
    pinMode(cs, OUTPUT);
    digitalWrite(cs, HIGH);
}

void SpiService::end() {
    SPI.end();
}

void SpiService::beginTransaction() {
    SPI.beginTransaction(SPISettings(spiFrequency, MSBFIRST, SPI_MODE0));
    digitalWrite(csPin, LOW);
}

void SpiService::endTransaction() {
    digitalWrite(csPin, HIGH);
    SPI.endTransaction();
}

uint8_t SpiService::transfer(uint8_t data) {
    return SPI.transfer(data);
}

std::string SpiService::readFlashID() {
    uint8_t id[3] = {0};

    beginTransaction();
    SPI.transfer(0x9F);  // JEDEC ID command
    for (uint8_t& byte : id) {
        byte = SPI.transfer(0x00);
    }
    endTransaction();

    char buf[32];
    snprintf(buf, sizeof(buf), "%02X %02X %02X", id[0], id[1], id[2]);
    return std::string(buf);
}

void SpiService::readFlashIdRaw(uint8_t* buffer) {
    beginTransaction();
    SPI.transfer(0x9F);  // JEDEC ID command
    for (int i = 0; i < 3; ++i) {
        buffer[i] = SPI.transfer(0x00);
    }
    endTransaction();
}

uint32_t SpiService::calculateFlashCapacity(uint8_t code) {
    // code 0x11 = 2^17 = 128 KB, etc.
    if (code >= 0x11 && code <= 0x20) {
        return 1UL << code;  // 2^code
    }
    return 0; // Non standard
}

void SpiService::readFlashData(uint32_t address, uint8_t* buffer, size_t length) {
    beginTransaction();
    SPI.transfer(0x03);  // Read Data command
    SPI.transfer((address >> 16) & 0xFF);
    SPI.transfer((address >> 8) & 0xFF);
    SPI.transfer(address & 0xFF);

    for (size_t i = 0; i < length; ++i) {
        buffer[i] = SPI.transfer(0x00);
    }
    endTransaction();
}

void SpiService::eraseSector(uint32_t address, uint32_t freq) {
    enableWrite(freq);  // 0x06

    SPI.beginTransaction(SPISettings(freq, MSBFIRST, SPI_MODE0));
    digitalWrite(csPin, LOW);

    SPI.transfer(0x20); // Sector erase
    SPI.transfer((address >> 16) & 0xFF);
    SPI.transfer((address >> 8) & 0xFF);
    SPI.transfer(address & 0xFF);

    digitalWrite(csPin, HIGH);
    SPI.endTransaction();

    waitForWriteComplete(freq);
}

void SpiService::enableWrite(uint32_t freq) {

    SPI.beginTransaction(SPISettings(freq, MSBFIRST, SPI_MODE0));
    digitalWrite(csPin, LOW);

    SPI.transfer(0x06); // Write Enable

    digitalWrite(csPin, HIGH);
    SPI.endTransaction();
}

void SpiService::waitForWriteComplete(uint32_t freq) {
    SPI.beginTransaction(SPISettings(freq, MSBFIRST, SPI_MODE0));
    digitalWrite(csPin, LOW);

    SPI.transfer(0x05); // Read Status Register
    while (true) {
        uint8_t status = SPI.transfer(0x00); // Dummy byte to receive status
        if ((status & 0x01) == 0) break;     // Wait until WIP bit is cleared
        delay(1);
    }

    digitalWrite(csPin, HIGH);
    SPI.endTransaction();
}

void SpiService::writeFlashPage(uint32_t address, const std::vector<uint8_t>& data, uint32_t freq) {
    const size_t maxPerPage = 256; // Page size (standard)

    size_t offset = 0;
    while (offset < data.size()) {
        size_t chunkSize = std::min(maxPerPage, data.size() - offset);

        enableWrite(freq);

        SPI.beginTransaction(SPISettings(freq, MSBFIRST, SPI_MODE0));
        digitalWrite(csPin, LOW);

        SPI.transfer(0x02); // Page Program
        SPI.transfer((address >> 16) & 0xFF);
        SPI.transfer((address >> 8) & 0xFF);
        SPI.transfer(address & 0xFF);

        for (size_t i = 0; i < chunkSize; ++i) {
            SPI.transfer(data[offset + i]);
        }

        digitalWrite(csPin, HIGH);
        SPI.endTransaction();

        waitForWriteComplete(freq);

        address += chunkSize;
        offset += chunkSize;
    }
}

void SpiService::writeFlashPatch(uint32_t address, const std::vector<uint8_t>& data, uint32_t freq) {
    const uint32_t sectorSize = 4096;
    uint32_t sectorStart = address & ~(sectorSize - 1);
    uint32_t offsetInSector = address - sectorStart;

    // Read the concerned sector
    std::vector<uint8_t> sectorData(sectorSize, 0xFF);
    readFlashData(sectorStart, sectorData.data(), sectorSize);

    // Modify data
    for (size_t i = 0; i < data.size(); ++i) {
        if ((offsetInSector + i) < sectorSize) {
            sectorData[offsetInSector + i] = data[i];
        }
    }

    // Erase the sector
    eraseSector(sectorStart, freq);

    // Write modified data
    for (uint32_t i = 0; i < sectorSize; i += 256) {
        std::vector<uint8_t> page(sectorData.begin() + i, sectorData.begin() + i + 256);
        writeFlashPage(sectorStart + i, page, freq);
    }
}

std::string SpiService::executeByteCode(const std::vector<ByteCode>& bytecodes) {
    std::string result;
    bool inTransaction = false;

    for (const auto& code : bytecodes) {
        switch (code.getCommand()) {
            case ByteCodeEnum::Start:
                if (!inTransaction) {
                    beginTransaction();
                    inTransaction = true;
                }
                break;

            case ByteCodeEnum::Stop:
                if (inTransaction) {
                    endTransaction();
                    inTransaction = false;
                }
                break;

            case ByteCodeEnum::Write:
                for (uint32_t i = 0; i < code.getRepeat(); ++i) {
                    transfer(code.getData());
                }
                break;

            case ByteCodeEnum::Read:
                for (uint32_t i = 0; i < code.getRepeat(); ++i) {
                    uint8_t val = transfer(0x00);  // dummy byte
                    char hex[5];
                    snprintf(hex, sizeof(hex), "%02X ", val);
                    result += hex;
                }
                break;

            case ByteCodeEnum::DelayMs:
                delay(code.getRepeat());
                break;

            case ByteCodeEnum::DelayUs:
                delayMicroseconds(code.getRepeat());
                break;

            default:
                break;
        }
    }

    // Close transaction if left open
    if (inTransaction) {
        endTransaction();
    }

    return result;
}

// #### SPI SLAVE ######

static ESP32SPISlave spiSlave;
static constexpr size_t SLAVE_BUFFER_SIZE = 8;
static std::atomic<bool> slave{false};

static uint8_t slave_tx_buf[SLAVE_BUFFER_SIZE] = {0};
static uint8_t slave_rx_buf[SLAVE_BUFFER_SIZE] = {0};

static std::deque<std::vector<uint8_t>> slaveBuffer;
static std::mutex slaveBufferMutex;

void IRAM_ATTR slaveTransactionCallback(spi_slave_transaction_t* trans, void* arg) {
    // Copy to vector
    size_t length_bytes = (trans->trans_len + 7) / 8; // trans_len in bits
    std::vector<uint8_t> received(slave_rx_buf, slave_rx_buf + length_bytes);

    {
        // Push thread safe
        std::lock_guard<std::mutex> lock(slaveBufferMutex);
        slaveBuffer.push_back(std::move(received));

        // Limit
        if (slaveBuffer.size() > 100) slaveBuffer.pop_front();
    }

    // Relaunch
    if (slave) {
        spiSlave.queue(slave_tx_buf, slave_rx_buf, SLAVE_BUFFER_SIZE);
        spiSlave.trigger();
    }
}

void SpiService::startSlave(int sclk, int miso, int mosi, int cs) {
    if (slave) return;
    slave = true;

    if (!slaveConfigured) {
        spiSlave.setDataMode(SPI_MODE0);
        spiSlave.setQueueSize(1);
        spiSlave.setUserPostTransCbAndArg(slaveTransactionCallback, nullptr);
        spiSlave.begin(FSPI, sclk, miso, mosi, cs);
        slaveConfigured = true;
    }

    // First launch then it loops until slave is true
    spiSlave.queue(slave_tx_buf, slave_rx_buf, SLAVE_BUFFER_SIZE);
    spiSlave.trigger();
}

void SpiService::stopSlave(int sclk, int miso, int mosi, int cs) {
    slave = false;
    slaveBuffer.clear();
    memset(slave_rx_buf, 0, SLAVE_BUFFER_SIZE);
    memset(slave_tx_buf, 0, SLAVE_BUFFER_SIZE);
    //Calling spiSlave.end() and then reallocate it cause crashes
}

bool SpiService::isSlave() const {
    return slave;
}

std::vector<std::vector<uint8_t>> SpiService::getSlaveData() {
    std::lock_guard<std::mutex> lock(slaveBufferMutex);
    std::vector<std::vector<uint8_t>> data(slaveBuffer.begin(), slaveBuffer.end());
    slaveBuffer.clear();
    return data;
}