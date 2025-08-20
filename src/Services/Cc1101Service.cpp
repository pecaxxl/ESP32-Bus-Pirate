#include "Cc1101Service.h"

void CC1101Service::configure(uint8_t mosi, uint8_t miso, uint8_t sclk, uint8_t cs, uint8_t gdo0, uint8_t gdo2, uint32_t frequency)
{
    //end();
    csPin = cs;
    spiFrequency = frequency;
    //SPI.begin(sclk, miso, mosi, cs);
    ELECHOUSE_cc1101.setSpiPin(sclk, miso, mosi, cs);
    ELECHOUSE_cc1101.setGDO(gdo0, gdo2);
    pinMode(cs, OUTPUT);
    digitalWrite(cs, HIGH);
    ELECHOUSE_cc1101.Init();              
    ELECHOUSE_cc1101.setGDO0(gdo0);         
    ELECHOUSE_cc1101.setCCMode(1);          
    ELECHOUSE_cc1101.setModulation(2);      
    ELECHOUSE_cc1101.setMHZ(433.92);        
    ELECHOUSE_cc1101.setDeviation(47.60);   
    ELECHOUSE_cc1101.setChannel(0);         
    ELECHOUSE_cc1101.setChsp(199.95);       
    ELECHOUSE_cc1101.setRxBW(812.50);       
    ELECHOUSE_cc1101.setDRate(9.6);         
    ELECHOUSE_cc1101.setPA(12);             
    ELECHOUSE_cc1101.setSyncMode(2);        
    ELECHOUSE_cc1101.setSyncWord(211, 145); 
    ELECHOUSE_cc1101.setAdrChk(0);          
    ELECHOUSE_cc1101.setAddr(0);            
    ELECHOUSE_cc1101.setWhiteData(0);       
    ELECHOUSE_cc1101.setPktFormat(0);       
    ELECHOUSE_cc1101.setLengthConfig(1);    
    ELECHOUSE_cc1101.setPacketLength(0);    
    ELECHOUSE_cc1101.setCrc(0);             
    ELECHOUSE_cc1101.setCRC_AF(0);          
    ELECHOUSE_cc1101.setDcFilterOff(0);     
    ELECHOUSE_cc1101.setManchester(0);      
    ELECHOUSE_cc1101.setFEC(0);             
    ELECHOUSE_cc1101.setPRE(0);             
    ELECHOUSE_cc1101.setPQT(0);             
    ELECHOUSE_cc1101.setAppendStatus(0);    
}

void CC1101Service::send(const std::string& msg) {
    ELECHOUSE_cc1101.SendData((char *)msg.c_str());
}

// void CC1101Service::setInput(uint8_t pin) {
//     pinMode(pin, INPUT);
// }

// void PinService::setInputPullup(uint8_t pin) {
//     pinMode(pin, INPUT_PULLUP);
// }

// void PinService::setOutput(uint8_t pin) {
//     pinMode(pin, OUTPUT);
// }

// void PinService::setHigh(uint8_t pin) {
//     setOutput(pin);  // force OUTPUT
//     digitalWrite(pin, HIGH);
// }

// void PinService::setLow(uint8_t pin) {
//     setOutput(pin);  // force OUTPUT
//     digitalWrite(pin, LOW);
// }

// bool PinService::read(uint8_t pin) {
//     return gpio_get_level((gpio_num_t)pin);
// }

// void PinService::togglePullup(uint8_t pin) {
//     bool enabled = pullupState[pin]; // default false if not set

//     if (enabled) {
//         setInput(pin);
//         pullupState[pin] = false;
//     } else {
//         setInputPullup(pin);
//         pullupState[pin] = true;
//     }
// }

// int PinService::readAnalog(uint8_t pin) {
//     pinMode(pin, INPUT); 
//     return analogRead(pin);
// }

// bool PinService::setupPwm(uint8_t pin, uint32_t freq, uint8_t dutyPercent) {
//     if (dutyPercent > 100) dutyPercent = 100;

//     int channel = pin % 16;
//     int resolution = 8;

//     if (!isPwmFeasible(freq, resolution))
//         return false;

//     bool ok = ledcSetup(channel, freq, resolution);
//     if (!ok) return false;

//     ledcAttachPin(pin, channel);
//     uint32_t dutyVal = (dutyPercent * ((1 << resolution) - 1)) / 100;
//     ledcWrite(channel, dutyVal);

//     return true;
// }

// bool PinService::isPwmFeasible(uint32_t freq, uint8_t resolutionBits) {
//     const uint32_t clkHz = 80000000; // horloge APB
//     const uint32_t maxDiv = 1 << 20;   // 20 bit prescaler max
//     if (resolutionBits > 20 || resolutionBits == 0) return false;

//     uint32_t divParam = clkHz / (freq * (1 << resolutionBits));
//     return divParam <= maxDiv && divParam > 0;
// }