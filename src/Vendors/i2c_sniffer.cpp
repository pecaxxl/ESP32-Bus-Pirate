/**
 * @AUTHOR Ákos Szabó (Whitehawk Tailor) - aaszabo@gmail.com
 * 
 * This is an I2C sniffer that logs traffic on I2C BUS.
 * 
 * It is not part of the I2C BUS. It is neither a Master, nor a Slave and puts no data to the lines.
 * It just listens and logs the communication.
 * 
 * Two pins as imput are attached to SDC and SDA lines.
 * Since the I2C communications runs on 400kHz so,
 * the tool that runs this program should be fast.
 * This was tested on an ESP32 bord Heltec WiFi Lora32 v2
 * ESP32 core runs on 240MHz.
 * It means there are 600 ESP32 cycles during one I2C clock tick.
 *
 * 
 * The program uses interrupts to detect
 * the raise edge of the SCL - bit transfer 
 * the falling edge of SDA if SCL is HIGH- START
 * the raise edge of SDA if SCL id HIGH - STOP 
 * 
 * In the interrupt routines there is just a few line of code
 * that mainly sets the status and stores the incoming bits.
 * Otherwise the program gets timeout panic in interrupt handler and
 * restart the CPU.
 *                    https://github.com/WhitehawkTailor/I2C-sniffer/
 */

#include "i2c_sniffer.h"

static uint8_t sniffer_scl_pin = 1;
static uint8_t sniffer_sda_pin = 2;
#define I2C_IDLE 0
#define I2C_TRX 2

static volatile byte i2cStatus = I2C_IDLE;
static uint32_t lastStartMillis = 0;
static volatile byte dataBuffer[9600];
static volatile uint16_t bufferPoiW = 0;
static uint16_t bufferPoiR = 0;
static volatile byte bitCount = 0;
static volatile uint16_t byteCount = 0;
static volatile byte i2cBitD = 0;
static volatile byte i2cBitD2 = 0;
static volatile byte i2cBitC = 0;
static volatile byte i2cClk = 0;
static volatile byte i2cAck = 0;
static volatile byte i2cCase = 0;
static volatile uint16_t falseStart = 0;
static volatile uint16_t sclUpCnt = 0;
static volatile uint16_t sdaUpCnt = 0;
static volatile uint16_t sdaDownCnt = 0;

void i2c_sniffer_begin(uint8_t scl, uint8_t sda) {
    sniffer_scl_pin = scl;
    sniffer_sda_pin = sda;
}

void IRAM_ATTR i2cTriggerOnRaisingSCL() {
    sclUpCnt++;
    if (i2cStatus == I2C_IDLE) {
        falseStart++;
    }

    i2cBitC = digitalRead(sniffer_sda_pin);
    i2cCase = 0;
    if (bitCount == 8) i2cCase = 1;
    if (bitCount == 7 && byteCount == 0) i2cCase = 2;

    bitCount++;

    switch (i2cCase) {
        case 0:
            dataBuffer[bufferPoiW++] = '0' + i2cBitC;
            break;
        case 1:
            dataBuffer[bufferPoiW++] = (i2cBitC ? '-' : '+');
            byteCount++;
            bitCount = 0;
            break;
        case 2:
            dataBuffer[bufferPoiW++] = (i2cBitC ? 'R' : 'W');
            break;
    }
}

void IRAM_ATTR i2cTriggerOnChangeSDA() {
    do {
        i2cBitD = digitalRead(sniffer_sda_pin);
        i2cBitD2 = digitalRead(sniffer_sda_pin);
    } while (i2cBitD != i2cBitD2);

    if (i2cBitD) {
        i2cClk = digitalRead(sniffer_scl_pin);
        if (i2cStatus != I2C_IDLE && i2cClk == 1) {
            i2cStatus = I2C_IDLE;
            bitCount = 0;
            byteCount = 0;
            bufferPoiW--;
            dataBuffer[bufferPoiW++] = 's';
            dataBuffer[bufferPoiW++] = '\n';
        }
        sdaUpCnt++;
    } else {
        i2cClk = digitalRead(sniffer_scl_pin);
        if (i2cStatus == I2C_IDLE && i2cClk) {
            i2cStatus = I2C_TRX;
            bitCount = 0;
            byteCount = 0;
            dataBuffer[bufferPoiW++] = 'S';
        }
        sdaDownCnt++;
    }
}

static void resetI2cVariable() {
    i2cStatus = I2C_IDLE;
    bufferPoiW = 0;
    bufferPoiR = 0;
    bitCount = 0;
    falseStart = 0;
}

static void processDataBuffer() {
    if (bufferPoiW == bufferPoiR) return;

    uint16_t pw = bufferPoiW;

    for (int i = bufferPoiR; i < pw; i++) {
        bufferPoiR++;
    }

    if (i2cStatus == I2C_IDLE && pw == bufferPoiW) {
        bufferPoiW = 0;
        bufferPoiR = 0;
    }
}

void i2c_sniffer_setup() {
    pinMode(sniffer_scl_pin, INPUT_PULLUP);
    pinMode(sniffer_sda_pin, INPUT_PULLUP);
    resetI2cVariable();
    attachInterrupt(sniffer_scl_pin, i2cTriggerOnRaisingSCL, RISING);
    attachInterrupt(sniffer_sda_pin, i2cTriggerOnChangeSDA, CHANGE);
}

void i2c_sniffer_stop() {
    detachInterrupt(sniffer_scl_pin);
    detachInterrupt(sniffer_sda_pin);
    i2cStatus = I2C_IDLE;
}

bool i2c_sniffer_available() {
    if (i2cStatus == I2C_IDLE) processDataBuffer();
    return bufferPoiR != bufferPoiW;
}

char i2c_sniffer_read() {
    if (bufferPoiR == bufferPoiW) return '\0';
    return dataBuffer[bufferPoiR++];
}

void i2c_sniffer_reset_buffer() {
    bufferPoiW = 0;
    bufferPoiR = 0;
    bitCount = 0;
    byteCount = 0;
}
