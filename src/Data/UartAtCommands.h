#pragma once
#include <vector>
#include <string>

// ===== Emoji safety (somme toolchains don't like UTF8) =====
#ifndef UART_AT_EMOJI
  #define UART_AT_EMOJI 1
#endif
#if UART_AT_EMOJI
  #define E(x) x
#else
  #define E(x) ""
#endif

// =============================================================

enum class AtStandard { Hayes, ThreeGPP, SIMCom, Quectel };

enum class AtArgType {
    String,     // free text
    Uint,       // unsigned integer
    Int,        // signed integer
    Bool01,     // 0/1
    Phone,      // +33....
    HexBytes,   // "01 AA 03 ..."
    Choice,     // must be in 'choices' (pipe-separated)
    Regex       // must match 'pattern'
};

struct AtActionArg {
    const char* name;           // prompt label
    bool        required;
    const char* defaultValue;   // default if user enters empty (optional)
    const char* hint;           // help text / example

    // typing + optional constraints
    AtArgType   type;
    long long   minValue;
    long long   maxValue;
    const char* choices;
    const char* pattern;
};

struct AtActionItem {
    const char* emoji;
    const char* label;
    const char* command;      // template: "AT+XXX=%1,\"%2\""
    AtStandard  standard;
    const char* example;
    bool        destructive;
    std::vector<AtActionArg> args;
};

enum class AtMode {
    Config, Info, SimPin, Network, Calls, Sms, Phonebook, Ussd, Data, TimeClock,
    TcpIp_Vendor, Http_Vendor, Gnss_Vendor, Lora, Audio, Power, Hardware, Mqtt, Files
};

struct AtModeOption {
    const char* emoji;
    const char* name;
    AtMode      mode;
};

// ================== Modes ==================

static const std::vector<AtModeOption> kModes = {
    { E("🛠️ "), "Config",                 AtMode::Config },
    { E("ℹ️ "), "Info",                   AtMode::Info },
    { E("🔐"), "SIM & PIN",               AtMode::SimPin },
    { E("📶"), "Telephony Network",       AtMode::Network },
    { E("📞"), "Calls",                   AtMode::Calls },
    { E("✉️ "), "SMS",                    AtMode::Sms },
    { E("📒"), "Phonebook",               AtMode::Phonebook },
    { E("💱"), "USSD",                    AtMode::Ussd },
    { E("🌐"), "Data (PDP)",              AtMode::Data },
    { E("⏰"), "Time & Clock",            AtMode::TimeClock },
    { E("🌍"), "Internet TCP/IP",         AtMode::TcpIp_Vendor },
    { E("🕸️ "), "HTTP(S)",                AtMode::Http_Vendor },
    { E("🧭"), "GNSS/GPS",                AtMode::Gnss_Vendor },
    { E("📡"), "LoRa / LoRaWAN",          AtMode::Lora },
    { E("🔈"), "Audio",                   AtMode::Audio },
    { E("🔋"), "Power / PSM",             AtMode::Power },
    { E("🧰"), "Hardware I/O",            AtMode::Hardware },
    { E("☁️ "), "MQTT",                   AtMode::Mqtt },
    { E("📁"), "File Storage",            AtMode::Files },
};

// ================== Actions per mode ==================

// CONFIG (universal)
static const std::vector<AtActionItem> kConfigActions = {
    { E("⚡"),"Test communication","AT", AtStandard::Hayes, "AT", false, {} },
    { E("🔈"),"Enable/Disable echo","ATE", AtStandard::Hayes, "ATE", false, {} },
    { E("🔇"),"Disable echo","ATE0", AtStandard::Hayes, "ATE0", false, {} },
    { E("🔊"),"Enable echo","ATE1", AtStandard::Hayes, "ATE1", false, {} },
    { E("🔄"), "Restart module", "AT+RST", AtStandard::Hayes, "AT+RST", false, {} },
    { E("💤"), "Deep sleep", "AT+GSLP=%1", AtStandard::Hayes, "AT+GSLP=1000", false,
        { { "Duration (ms)", true, "1000", "sleep time (module-dependent)",
            AtArgType::Uint, 1, 86400000, nullptr, nullptr } } },
    { E("🧼"), "Factory reset (ESP-AT)", "AT+RESTORE", AtStandard::Hayes, "AT+RESTORE", true, {} },

    // UART configuration (ESP-AT)
    { E("🛠️ "), "UART config (deprecated)", "AT+UART=%1,%2,%3,%4,%5", AtStandard::Hayes,
      "AT+UART=115200,8,1,0,0", false,
        { { "Baud",      true, "115200", nullptr,               AtArgType::Uint,   1200, 921600, nullptr, nullptr },
          { "Data bits", true, "8",      "5/6/7/8",             AtArgType::Choice, 0,    0,      "5|6|7|8", nullptr },
          { "Stop bits", true, "1",      "1/2",                 AtArgType::Choice, 0,    0,      "1|2",    nullptr },
          { "Parity",    true, "0",      "0=None,1=Odd,2=Even", AtArgType::Choice, 0,    0,      "0|1|2",  nullptr },
          { "FlowCtl",   true, "0",      "0=off,1=on",          AtArgType::Choice, 0,    0,      "0|1",    nullptr } } },

    { E("🛠️ "), "UART current (no save)", "AT+UART_CUR=%1,%2,%3,%4,%5", AtStandard::Hayes,
      "AT+UART_CUR=115200,8,1,0,0", false,
        { { "Baud",      true, "115200", nullptr,               AtArgType::Uint,   1200, 921600, nullptr, nullptr },
          { "Data bits", true, "8",      "5/6/7/8",             AtArgType::Choice, 0,    0,      "5|6|7|8", nullptr },
          { "Stop bits", true, "1",      "1/2",                 AtArgType::Choice, 0,    0,      "1|2",    nullptr },
          { "Parity",    true, "0",      "0=None,1=Odd,2=Even", AtArgType::Choice, 0,    0,      "0|1|2",  nullptr },
          { "FlowCtl",   true, "0",      "0=off,1=on",          AtArgType::Choice, 0,    0,      "0|1",    nullptr } } },

    { E("🛠️ "), "UART default (save)", "AT+UART_DEF=%1,%2,%3,%4,%5", AtStandard::Hayes,
      "AT+UART_DEF=115200,8,1,0,0", false,
        { { "Baud",      true, "115200", nullptr,               AtArgType::Uint,   1200, 921600, nullptr, nullptr },
          { "Data bits", true, "8",      "5/6/7/8",             AtArgType::Choice, 0,    0,      "5|6|7|8", nullptr },
          { "Stop bits", true, "1",      "1/2",                 AtArgType::Choice, 0,    0,      "1|2",    nullptr },
          { "Parity",    true, "0",      "0=None,1=Odd,2=Even", AtArgType::Choice, 0,    0,      "0|1|2",  nullptr },
          { "FlowCtl",   true, "0",      "0=off,1=on",          AtArgType::Choice, 0,    0,      "0|1",    nullptr } } },

    { E("🌙"), "Sleep mode", "AT+SLEEP=%1", AtStandard::Hayes, "AT+SLEEP=1", false,
        { { "Mode", true, "1", "0=off,1=light,2=modem (module-dep.)",
            AtArgType::Choice, 0, 0, "0|1|2", nullptr } } },

    { E("📶"), "Set RF TX power", "AT+RFPOWER=%1", AtStandard::Hayes, "AT+RFPOWER=78", false,
        { { "Level", true, "78", "module-dependent", AtArgType::Uint, 0, 100, nullptr, nullptr } } },

    { E("🔌"), "RF power by VDD", "AT+RFVDD=%1", AtStandard::Hayes, "AT+RFVDD=1", false,
        { { "Enable", true, "1", "0/1", AtArgType::Bool01, 0, 0, nullptr, nullptr } } },

    { E("♻️ "),"Soft reset (profile)","ATZ", AtStandard::Hayes, "ATZ", false, {} },
    { E("🏭"),"Factory defaults","AT&F", AtStandard::Hayes, "AT&F", true, {} },
    { E("💾"),"Save profile","AT&W", AtStandard::Hayes, "AT&W", false, {} },
    { E("⏩"),"Set baud rate","AT+IPR=%1", AtStandard::ThreeGPP, "AT+IPR=115200", false,
        { { "Baud rate", true, "115200", "9600 / 19200 / 115200 ...",
            AtArgType::Uint, 1200, 921600, nullptr, nullptr } } },
    { E("🧩"),"Verbose errors","AT+CMEE=%1", AtStandard::ThreeGPP, "AT+CMEE=2", false,
        { { "Mode", true, "2", "0=off,1=basic,2=verbose",
            AtArgType::Choice, 0, 0, "0|1|2", nullptr } } },
    { E("🔤"),"Char set","AT+CSCS=\"%1\"", AtStandard::ThreeGPP, "AT+CSCS=\"GSM\"", false,
        { { "Charset", true, "GSM", "GSM/IRA/UCS2",
            AtArgType::Choice, 0, 0, "GSM|IRA|UCS2", nullptr } } },
    { E("↔️ "),"Flow control","AT+IFC=%1,%2", AtStandard::ThreeGPP, "AT+IFC=2,2", false,
        { { "RX flow", true, "2", "0=off,1=hw,2=sw", AtArgType::Choice, 0, 0, "0|1|2", nullptr },
          { "TX flow", true, "2", "0=off,1=hw,2=sw", AtArgType::Choice, 0, 0, "0|1|2", nullptr } } },

    { E("🔍"), "View active config", "AT&V", AtStandard::Hayes, "AT&V", false, {} },
    { E("🧭"), "Power-up profile", "AT&Y%1", AtStandard::Hayes, "AT&Y1", false,
    { { "Profile", true, "1", "0 or 1", AtArgType::Choice, 0,0, "0|1", nullptr } } },
    { E("🤫"), "Suppress result codes", "ATQ%1", AtStandard::Hayes, "ATQ0", false,
    { { "Quiet", true, "0", "0=off,1=on", AtArgType::Choice, 0,0, "0|1", nullptr } } },
    { E("🗣️ "), "Response format", "ATV%1", AtStandard::Hayes, "ATV1", false,
    { { "Mode", true, "1", "0=numeric,1=verbose", AtArgType::Choice, 0,0, "0|1", nullptr } } },
    { E("📎"), "DCD usage", "AT&C%1", AtStandard::Hayes, "AT&C1", false,
    { { "Mode", true, "1", "0=always ON,1=follows carrier", AtArgType::Choice, 0,0, "0|1", nullptr } } },
    { E("🎚️ "), "DTR behaviour", "AT&D%1", AtStandard::Hayes, "AT&D2", false,
    { { "Mode", true, "2", "0=ignore,2=hang up+CMD", AtArgType::Choice, 0,0, "0|2", nullptr } } },
    { E("🧩"), "Char framing (ICF)", "AT+ICF=%1,%2", AtStandard::ThreeGPP, "AT+ICF=3,1", false,
    { { "Format", true, "3", "module-specific", AtArgType::Uint, 0,10, nullptr, nullptr },
        { "Parity", true, "0", "0=none,1=odd,2=even (module)", AtArgType::Uint, 0,5, nullptr, nullptr } } },

};

// INFO (universal)
static const std::vector<AtActionItem> kInfoActions = {
    { E("ℹ️ "),"Module info","ATI", AtStandard::Hayes, "ATI", false, {} },
    { E("🏷️ "),"Manufacturer","AT+GMI", AtStandard::Hayes, "AT+GMI", false, {} },
    { E("🔧"),"Model","AT+GMM", AtStandard::Hayes, "AT+GMM", false, {} },
    { E("🛠️ "),"Firmware version","AT+GMR", AtStandard::Hayes, "AT+GMR", false, {} },
    { E("🆔"),  "IMEI (GSN)",          "AT+GSN",  AtStandard::Hayes,    "AT+GSN",  false, {} },
    { E("🆔"),"IMEI","AT+CGSN", AtStandard::ThreeGPP, "AT+CGSN", false, {} },
    { E("🧩"),"Capabilities","AT+GCAP", AtStandard::Hayes, "AT+GCAP", false, {} },
    { E("🏷️ "), "Manufacturer (CGMI)", "AT+CGMI", AtStandard::ThreeGPP, "AT+CGMI", false, {} },
    { E("🔧"),  "Model (CGMM)",        "AT+CGMM", AtStandard::ThreeGPP, "AT+CGMM", false, {} },
    { E("🛠️ "), "Firmware (CGMR)",     "AT+CGMR", AtStandard::ThreeGPP, "AT+CGMR", false, {} },

};

// SIM & PIN
static const std::vector<AtActionItem> kSimPinActions = {
    { E("🔍"),"PIN status","AT+CPIN?", AtStandard::ThreeGPP, "AT+CPIN?", false, {} },
    { E("🔓"),"Enter PIN/PUK","AT+CPIN=\"%1\"", AtStandard::ThreeGPP, "AT+CPIN=\"1234\"", true,
        { { "PIN or PUK", true, nullptr, "1234 / PUK code", AtArgType::String, 0, 0, nullptr, nullptr } } },
    { E("🔐"),"Enable SIM lock","AT+CLCK=\"SC\",1,\"%1\"", AtStandard::ThreeGPP, "AT+CLCK=\"SC\",1,\"1234\"", true,
        { { "PIN", true, nullptr, "current PIN", AtArgType::String, 0, 0, nullptr, nullptr } } },
    { E("🔓"),"Disable SIM lock","AT+CLCK=\"SC\",0,\"%1\"", AtStandard::ThreeGPP, "AT+CLCK=\"SC\",0,\"1234\"", true,
        { { "PIN", true, nullptr, "current PIN", AtArgType::String, 0, 0, nullptr, nullptr } } },
    { E("♻️ "),"Change PIN","AT+CPWD=\"SC\",\"%1\",\"%2\"", AtStandard::ThreeGPP, "AT+CPWD=\"SC\",\"1234\",\"0000\"", true,
        { { "Old PIN", true, nullptr, nullptr, AtArgType::String, 0, 0, nullptr, nullptr },
          { "New PIN", true, nullptr, nullptr, AtArgType::String, 0, 0, nullptr, nullptr } } },
};

// TEL NETWORK
static const std::vector<AtActionItem> kNetworkActions = {
    { E("📡"),"Signal quality","AT+CSQ", AtStandard::ThreeGPP, "AT+CSQ", false, {} },
    { E("🗺️ "),"GSM reg status","AT+CREG?", AtStandard::ThreeGPP, "AT+CREG?", false, {} },
    { E("🗺️ "),"Packet reg status","AT+CGREG?", AtStandard::ThreeGPP, "AT+CGREG?", false, {} },
    { E("🗺️ "),"LTE reg status","AT+CEREG?", AtStandard::ThreeGPP, "AT+CEREG?", false, {} },
    { E("🏢"),"Current operator","AT+COPS?", AtStandard::ThreeGPP, "AT+COPS?", false, {} },
    { E("🧭"),"List operators","AT+COPS=?", AtStandard::ThreeGPP, "AT+COPS=?", false, {} },
    { E("📶"), "Extended signal quality", "AT+CESQ", AtStandard::ThreeGPP, "AT+CESQ", false, {} },
    { E("🔁"),"Auto select operator","AT+COPS=0", AtStandard::ThreeGPP, "AT+COPS=0", false, {} },
    { E("🧯"), "Extended error report", "AT+CEER", AtStandard::ThreeGPP, "AT+CEER", false, {} },
    { E("📶"),"Set operator","AT+COPS=1,2,\"%1\"", AtStandard::ThreeGPP, "AT+COPS=1,2,\"20801\"", false,
        { { "MCCMNC", true, nullptr, "e.g. 20801",
            AtArgType::Regex, 0, 0, nullptr, "^[0-9]{5,6}$" } } },
    { E("🗺️ "), "Network mode", "AT+CNMP=%1", AtStandard::ThreeGPP, "AT+CNMP=2", false,
        { { "Mode", true, "2", "module-dependent", AtArgType::Uint, 0, 100, nullptr, nullptr } } },
    { E("🗺️ "), "Network scan mode", "AT+QCFG=\"nwscanmode\",%1", AtStandard::Quectel, "AT+QCFG=\"nwscanmode\",3", false,
        { { "Mode", true, "3", "0=auto,3=GSM/WCDMA/LTE", AtArgType::Uint, 0, 10, nullptr, nullptr } } },
    { E("📌"),"Manual operator (PLMN)","AT+COPS=1,2,\"%1\"", AtStandard::ThreeGPP, "AT+COPS=1,2,\"20801\"", false,
        { { "MCCMNC", true, nullptr, "e.g. 20801",
            AtArgType::Regex, 0, 0, nullptr, "^[0-9]{5,6}$" } } },
    { E("🛰️ "),"Function level","AT+CFUN=%1", AtStandard::ThreeGPP, "AT+CFUN=1", false,
        { { "CFUN", true, "1", "0=minimal,1=full,4=airplane",
            AtArgType::Choice, 0, 0, "0|1|4", nullptr } } },

    // Vendor (popular)
    { E("📶"),"RAT mode (SIMCom)","AT+CNMP=%1", AtStandard::SIMCom, "AT+CNMP=2", false,
        { { "Mode", true, "2", "2=auto,13=GSM,38=LTE…", AtArgType::Uint, 0, 100, nullptr, nullptr } } },
    { E("🧱"),"NB/LTE pref (SIMCom)","AT+CMNB=%1", AtStandard::SIMCom, "AT+CMNB=1", false,
        { { "Pref", true, "1", "module-dependent", AtArgType::Uint, 0, 10, nullptr, nullptr } } },
    { E("🧭"),"Scan mode (Quectel)","AT+QCFG=\"nwscanmode\",%1", AtStandard::Quectel, "AT+QCFG=\"nwscanmode\",3", false,
        { { "Mode", true, "3", "0=auto,3=GSM/WCDMA/LTE", AtArgType::Uint, 0, 10, nullptr, nullptr } } },
};

// CALLS
static const std::vector<AtActionItem> kCallsActions = {
    { E("📲"),"Dial (voice)","ATD%1;", AtStandard::Hayes, "ATD+33123456789;", false,
        { { "Number", true, nullptr, "+33...", AtArgType::Phone, 0, 0, nullptr, nullptr } } },
    { E("✅"),"Answer","ATA", AtStandard::Hayes, "ATA", false, {} },
    { E("📴"),"Hang up","ATH", AtStandard::Hayes, "ATH", false, {} },
    { E("📃"),"List current calls","AT+CLCC", AtStandard::ThreeGPP, "AT+CLCC", false, {} },
    { E("🆔"),"Caller ID presentation","AT+CLIP=%1", AtStandard::ThreeGPP, "AT+CLIP=1", false,
        { { "CLIP", true, "1", "0/1", AtArgType::Bool01, 0, 0, nullptr, nullptr } } },
    { E("🙈"),"Caller ID restriction","AT+CLIR=%1", AtStandard::ThreeGPP, "AT+CLIR=1", false,
        { { "CLIR", true, "1", "0/1", AtArgType::Bool01, 0, 0, nullptr, nullptr } } },
    { E("🔊"),"DTMF tone","AT+VTS=\"%1\"", AtStandard::ThreeGPP, "AT+VTS=\"123#\"", false,
        { { "Digits", true, nullptr, "0-9 * #",
            AtArgType::Regex, 0, 0, nullptr, "^[0-9*#]+$" } } },
    { E("⏱️ "), "DTMF duration", "AT+VTD=%1", AtStandard::ThreeGPP, "AT+VTD=4", false,
        { { "Units", true, "4", "1..255 (dépend module)", AtArgType::Uint, 1, 255, nullptr, nullptr } } },
    { E("👁️ "), "Connected line ID (COLP)", "AT+COLP=%1", AtStandard::ThreeGPP, "AT+COLP=1", false,
        { { "Enable", true, "1", "0/1", AtArgType::Bool01, 0, 0, nullptr, nullptr } } },
    { E("👁️ "), "Connected line ID (COLP)", "AT+COLP=%1", AtStandard::ThreeGPP, "AT+COLP=1", false,
        { { "Enable", true, "1", "0/1", AtArgType::Bool01, 0,0, nullptr, nullptr } } },
    { E("🏷️ "), "Default dial TOA", "AT+CSTA=%1", AtStandard::ThreeGPP, "AT+CSTA=145", false,
        { { "Type", true, "145", "129=national,145=international", AtArgType::Choice, 0,0, "129|145", nullptr } } },
    { E("📱"), "Phone activity status", "AT+CPAS", AtStandard::ThreeGPP, "AT+CPAS", false, {} },

};

// SMS
static const std::vector<AtActionItem> kSmsActions = {
    { E("🧰"),"PDU mode","AT+CMGF=0", AtStandard::ThreeGPP, "AT+CMGF=0", false, {} },
    { E("✉️"),"Text mode","AT+CMGF=1", AtStandard::ThreeGPP, "AT+CMGF=1", false, {} },
    { E("📮"),"SMSC address","AT+CSCA?", AtStandard::ThreeGPP, "AT+CSCA?", false, {} },
    { E("📤"),"Send SMS (text)","AT+CMGS=\"%1\"", AtStandard::ThreeGPP, "AT+CMGS=\"+33123456789\"", false,
        { { "Phone number", true, nullptr, "+33...", AtArgType::Phone, 0, 0, nullptr, nullptr } } },
    { E("📥"),"Read SMS","AT+CMGR=%1", AtStandard::ThreeGPP, "AT+CMGR=1", false,
        { { "Index", true, "1", "message index", AtArgType::Uint, 1, 1000, nullptr, nullptr } } },
    { E("📜"),"List SMS","AT+CMGL=\"%1\"", AtStandard::ThreeGPP, "AT+CMGL=\"ALL\"", false,
        { { "Filter", true, "ALL", "REC UNREAD|REC READ|STO UNSENT|STO SENT|ALL",
            AtArgType::Choice, 0, 0, "REC UNREAD|REC READ|STO UNSENT|STO SENT|ALL", nullptr } } },
    { E("🗑️ "),"Delete SMS","AT+CMGD=%1", AtStandard::ThreeGPP, "AT+CMGD=1", true,
        { { "Index", true, nullptr, "message index", AtArgType::Uint, 0, 1000, nullptr, nullptr } } },
    { E("🔔"),"New SMS indications","AT+CNMI=%1,%2,%3,%4,%5", AtStandard::ThreeGPP, "AT+CNMI=2,1,0,0,0", false,
        { { "mode", true, "2", nullptr, AtArgType::Uint, 0, 3, nullptr, nullptr },
          { "mt",   true, "1", nullptr, AtArgType::Uint, 0, 3, nullptr, nullptr },
          { "bm",   true, "0", nullptr, AtArgType::Uint, 0, 3, nullptr, nullptr },
          { "ds",   true, "0", nullptr, AtArgType::Uint, 0, 3, nullptr, nullptr },
          { "bfr",  true, "0", nullptr, AtArgType::Uint, 0, 3, nullptr, nullptr } } },
    { E("⚙️ "), "SMS params (CSMP)", "AT+CSMP=%1,%2,%3,%4", AtStandard::ThreeGPP, "AT+CSMP=17,167,0,0", false,
    { { "FO", true, "17", nullptr, AtArgType::Uint, 0, 255, nullptr, nullptr },
        { "VP", true, "167", nullptr, AtArgType::Uint, 0, 255, nullptr, nullptr },
        { "PID",true, "0", nullptr, AtArgType::Uint, 0, 255, nullptr, nullptr },
        { "DCS",true, "0", nullptr, AtArgType::Uint, 0, 255, nullptr, nullptr } } },

    { E("💾"), "Write SMS to mem", "AT+CMGW=\"%1\"", AtStandard::ThreeGPP, "AT+CMGW=\"+33123456789\"", false,
    { { "Destination (opt.)", false, nullptr, "Could be empty for draft", AtArgType::Phone, 0, 0, nullptr, nullptr } } },

    { E("📤"), "Send SMS from mem", "AT+CMSS=%1,\"%2\"", AtStandard::ThreeGPP, "AT+CMSS=1,\"+33123456789\"", false,
    { { "Index", true, "1", nullptr, AtArgType::Uint, 1, 1000, nullptr, nullptr },
        { "Override number (opt.)", false, nullptr, "optionnel", AtArgType::Phone, 0, 0, nullptr, nullptr } } },

    { E("✅"), "Ack incoming SMS", "AT+CNMA", AtStandard::ThreeGPP, "AT+CNMA", false, {} },

    { E("📦"), "Storage status", "AT+CPMS?", AtStandard::ThreeGPP, "AT+CPMS?", false, {} },

    { E("🧭"), "Select message service", "AT+CSMS=%1", AtStandard::ThreeGPP, "AT+CSMS=1", false,
    { { "Service", true, "1", "module-dependent", AtArgType::Uint, 0, 3, nullptr, nullptr } } },
    { E("🔎"), "Show text params", "AT+CSDH=%1", AtStandard::ThreeGPP, "AT+CSDH=1", false,
    { { "Show", true, "1", "0/1", AtArgType::Bool01, 0,0, nullptr, nullptr } } },
    { E("📢"), "Cell broadcast filter", "AT+CSCB=%1,\"%2\",\"%3\"", AtStandard::ThreeGPP, "AT+CSCB=0,\"\",\"\"", false,
    { { "Mode", true, "0", "0=accept all,1=reject", AtArgType::Choice, 0,0, "0|1", nullptr },
        { "MIDs", true, "", "IDs list", AtArgType::String, 0,0, nullptr, nullptr },
        { "DCS",  true, "", "DCS list", AtArgType::String, 0,0, nullptr, nullptr } } },
    { E("💾"), "Save SMS settings", "AT+CSAS", AtStandard::ThreeGPP, "AT+CSAS", false, {} },
    { E("♻️"), "Restore SMS settings", "AT+CRES", AtStandard::ThreeGPP, "AT+CRES", false, {} },


            
};

// PHONEBOOK
static const std::vector<AtActionItem> kPhonebookActions = {
    { E("📚"),"Select storage","AT+CPBS=\"%1\"", AtStandard::ThreeGPP, "AT+CPBS=\"SM\"", false,
        { { "Storage", true, "SM", "SM/ME", AtArgType::Choice, 0, 0, "SM|ME", nullptr } } },
    { E("📖"),"Read entries","AT+CPBR=%1,%2", AtStandard::ThreeGPP, "AT+CPBR=1,10", false,
        { { "Start index", true, "1", nullptr, AtArgType::Uint, 1, 250, nullptr, nullptr },
          { "End index",   true, "10", nullptr, AtArgType::Uint, 1, 250, nullptr, nullptr } } },
    { E("🔎"),"Find by name","AT+CPBF=\"%1\"", AtStandard::ThreeGPP, "AT+CPBF=\"ALICE\"", false,
        { { "Query", true, nullptr, "substring", AtArgType::String, 0, 0, nullptr, nullptr } } },
    { E("✏️ "),"Write/Update entry","AT+CPBW=%1,\"%2\",%3,\"%4\"", AtStandard::ThreeGPP,
      "AT+CPBW=1,\"+33123456789\",145,\"ALICE\"", true,
        { { "Index",  true, nullptr, nullptr, AtArgType::Uint, 1, 250, nullptr, nullptr },
          { "Number", true, nullptr, "+33...", AtArgType::Phone, 0, 0, nullptr, nullptr },
          { "Type",   true, "145", "129=national,145=international", AtArgType::Choice, 0, 0, "129|145", nullptr },
          { "Name",   true, nullptr, nullptr, AtArgType::String, 0, 0, nullptr, nullptr } } },
};

// USSD
static const std::vector<AtActionItem> kUssdActions = {
    { E("💱"),"USSD request","AT+CUSD=1,\"%1\",%2", AtStandard::ThreeGPP, "AT+CUSD=1,\"*100#\",15", false,
        { { "USSD code", true, nullptr, "*100#", AtArgType::Regex, 0, 0, nullptr, "^[*#0-9]+#$" },
          { "DCS",       true, "15", "15=GSM 7-bit", AtArgType::Uint, 0, 255, nullptr, nullptr } } },
    { E("🛑"),"Cancel USSD","AT+CUSD=2", AtStandard::ThreeGPP, "AT+CUSD=2", false, {} },
};

// DATA / PDP
static const std::vector<AtActionItem> kDataActions = {
    { E("🔌"),"Attach packet svc","AT+CGATT=%1", AtStandard::ThreeGPP, "AT+CGATT=1", false,
        { { "Attach", true, "1", "0/1", AtArgType::Bool01, 0, 0, nullptr, nullptr } } },
    { E("🔒"), "PDP auth (PAP/CHAP)", "AT+CGAUTH=%1,%2,\"%3\",\"%4\"",
        AtStandard::ThreeGPP, "AT+CGAUTH=1,1,\"user\",\"pass\"", false,
        { { "CID",  true, "1", nullptr, AtArgType::Uint, 1, 16, nullptr, nullptr },
            { "Type", true, "1", "0=None,1=PAP,2=CHAP", AtArgType::Choice, 0, 0, "0|1|2", nullptr },
            { "User", true, nullptr, nullptr, AtArgType::String, 0, 0, nullptr, nullptr },
            { "Pass", true, nullptr, nullptr, AtArgType::String, 0, 0, nullptr, nullptr } } },
    { E("📑"),"Define PDP context","AT+CGDCONT=%1,\"IP\",\"%2\"", AtStandard::ThreeGPP,
      "AT+CGDCONT=1,\"IP\",\"internet\"", false,
        { { "CID", true, "1", nullptr, AtArgType::Uint, 1, 16, nullptr, nullptr },
          { "APN", true, "internet", "APN name", AtArgType::Regex, 0, 0, nullptr, "^[A-Za-z0-9._-]+$" } } },
    { E("▶️ "),"Activate PDP","AT+CGACT=1,%1", AtStandard::ThreeGPP, "AT+CGACT=1,1", false,
        { { "CID", true, "1", nullptr, AtArgType::Uint, 1, 16, nullptr, nullptr } } },
    { E("⏹️ "),"Deactivate PDP","AT+CGACT=0,%1", AtStandard::ThreeGPP, "AT+CGACT=0,1", false,
        { { "CID", true, "1", nullptr, AtArgType::Uint, 1, 16, nullptr, nullptr } } },
    { E("🌍"),"Show IP address","AT+CGPADDR=%1", AtStandard::ThreeGPP, "AT+CGPADDR=1", false,
        { { "CID", true, "1", nullptr, AtArgType::Uint, 1, 16, nullptr, nullptr } } },
    { E("📘"),"Context readback","AT+CGCONTRDP=%1", AtStandard::ThreeGPP, "AT+CGCONTRDP=1", false,
        { { "CID (or empty)", false, nullptr, "leave empty for all", AtArgType::String, 0, 0, nullptr, nullptr } } },
};

// TIME & CLOCK
static const std::vector<AtActionItem> kTimeClockActions = {
    { E("⏰"),"Read clock","AT+CCLK?", AtStandard::ThreeGPP, "AT+CCLK?", false, {} },
    { E("🕒"),"Set clock","AT+CCLK=\"%1\"", AtStandard::ThreeGPP, "AT+CCLK=\"25/08/11,12:00:00+08\"", true,
        { { "Datetime (yy/MM/dd,hh:mm:ss±tz)", true, nullptr, "25/08/11,12:00:00+08",
            AtArgType::Regex, 0, 0, nullptr, "^[0-9]{2}/[0-9]{2}/[0-9]{2},[0-9]{2}:[0-9]{2}:[0-9]{2}[+-][0-9]{2}$" } } },
    { E("🛰️ "), "Auto TZ update (NITZ)", "AT+CTZU=%1", AtStandard::ThreeGPP, "AT+CTZU=1", false,
    { { "Enable", true, "1", "0/1", AtArgType::Bool01, 0, 0, nullptr, nullptr } } },

    { E("🛰️ "), "TZ URC reporting", "AT+CTZR=%1", AtStandard::ThreeGPP, "AT+CTZR=1", false,
    { { "Enable", true, "1", "0/1", AtArgType::Bool01, 0, 0, nullptr, nullptr } } },

};

// INTERNET TCP/IP
static const std::vector<AtActionItem> kTcpIpActions = {
 // --- Wi-Fi (ESP-AT) additions (no duplicates) ---
    { E("📶"), "Wi-Fi mode (STA/AP)", "AT+CWMODE=%1", AtStandard::Hayes, "AT+CWMODE=1", false,
        { { "Mode", true, "1", "1=STA,2=AP,3=STA+AP", AtArgType::Choice, 0, 0, "1|2|3", nullptr } } },
    { E("🔎"), "Wi-Fi scan", "AT+CWLAP", AtStandard::Hayes, "AT+CWLAP", false, {} },
    { E("🔗"), "Wi-Fi join AP", "AT+CWJAP=\"%1\",\"%2\"", AtStandard::Hayes, "AT+CWJAP=\"MySSID\",\"MyPass\"", false,
        { { "SSID", true, nullptr, "network SSID", AtArgType::String, 0, 0, nullptr, nullptr },
          { "Password", true, nullptr, "network password", AtArgType::String, 0, 0, nullptr, nullptr } } },
    { E("🧹"), "Wi-Fi disconnect", "AT+CWQAP", AtStandard::Hayes, "AT+CWQAP", false, {} },
    { E("🌐"),"IP local addr","AT+CIFSR", AtStandard::SIMCom, "AT+CIFSR", false, {} },
    { E("📶"), "Wi-Fi mode (DEF)", "AT+CWMODE_DEF=%1", AtStandard::Hayes, "AT+CWMODE_DEF=1", false,
        { { "Mode", true, "1", "1=STA,2=AP,3=STA+AP", AtArgType::Choice, 0, 0, "1|2|3", nullptr } } },

    { E("🔗"), "Wi-Fi join AP (CUR)", "AT+CWJAP_CUR=\"%1\",\"%2\"", AtStandard::Hayes, "AT+CWJAP_CUR=\"MySSID\",\"MyPass\"", false,
        { { "SSID", true, nullptr, "network SSID", AtArgType::String, 0, 0, nullptr, nullptr },
          { "Password", true, nullptr, "network password", AtArgType::String, 0, 0, nullptr, nullptr } } },
    { E("🔗"), "Wi-Fi join AP (DEF)", "AT+CWJAP_DEF=\"%1\",\"%2\"", AtStandard::Hayes, "AT+CWJAP_DEF=\"MySSID\",\"MyPass\"", false,
        { { "SSID", true, nullptr, "network SSID", AtArgType::String, 0, 0, nullptr, nullptr },
          { "Password", true, nullptr, "network password", AtArgType::String, 0, 0, nullptr, nullptr } } },

    { E("🛜"), "SoftAP config (CUR)", "AT+CWSAP_CUR=\"%1\",\"%2\",%3,%4", AtStandard::Hayes,
      "AT+CWSAP_CUR=\"MyAP\",\"apPass\",6,4", false,
        { { "SSID",    true, "MyAP",  nullptr, AtArgType::String, 0, 0, nullptr, nullptr },
          { "Password",true, "apPass",nullptr, AtArgType::String, 0, 0, nullptr, nullptr },
          { "Channel", true, "6",     nullptr, AtArgType::Uint,   1, 13,   nullptr, nullptr },
          { "Ecn",     true, "4",     "0=OPEN,1=WEP,2=WPA_PSK,3=WPA2_PSK,4=WPA_WPA2_PSK",
                                     AtArgType::Choice, 0, 0, "0|1|2|3|4", nullptr } } },
    { E("🛜"), "SoftAP config (DEF)", "AT+CWSAP_DEF=\"%1\",\"%2\",%3,%4", AtStandard::Hayes,
      "AT+CWSAP_DEF=\"MyAP\",\"apPass\",6,4", false,
        { { "SSID",    true, "MyAP",  nullptr, AtArgType::String, 0, 0, nullptr, nullptr },
          { "Password",true, "apPass",nullptr, AtArgType::String, 0, 0, nullptr, nullptr },
          { "Channel", true, "6",     nullptr, AtArgType::Uint,   1, 13,   nullptr, nullptr },
          { "Ecn",     true, "4",     "0=OPEN,1=WEP,2=WPA_PSK,3=WPA2_PSK,4=WPA_WPA2_PSK",
                                     AtArgType::Choice, 0, 0, "0|1|2|3|4", nullptr } } },

    { E("👥"), "List STA on softAP", "AT+CWLIF", AtStandard::Hayes, "AT+CWLIF", false, {} },

    { E("⚙️ "), "DHCP (CUR)", "AT+CWDHCP_CUR=%1,%2", AtStandard::Hayes, "AT+CWDHCP_CUR=1,1", false,
        { { "Mode", true, "1", "0=STA,1=AP,2=BOTH", AtArgType::Choice, 0, 0, "0|1|2", nullptr },
          { "Enable", true, "1", "0/1", AtArgType::Bool01, 0, 0, nullptr, nullptr } } },
    { E("⚙️ "), "DHCP (DEF)", "AT+CWDHCP_DEF=%1,%2", AtStandard::Hayes, "AT+CWDHCP_DEF=1,1", false,
        { { "Mode", true, "1", "0=STA,1=AP,2=BOTH", AtArgType::Choice, 0, 0, "0|1|2", nullptr },
          { "Enable", true, "1", "0/1", AtArgType::Bool01, 0, 0, nullptr, nullptr } } },

    { E("🔁"), "Auto-connect on boot", "AT+CWAUTOCONN=%1", AtStandard::Hayes, "AT+CWAUTOCONN=1", false,
        { { "Enable", true, "1", "0/1", AtArgType::Bool01, 0, 0, nullptr, nullptr } } },

    { E("🆔"), "STA MAC (CUR)", "AT+CIPSTAMAC_CUR=\"%1\"", AtStandard::Hayes, "AT+CIPSTAMAC_CUR=\"AA:BB:CC:DD:EE:FF\"", false,
        { { "MAC", true, nullptr, "AA:BB:CC:DD:EE:FF", AtArgType::Regex, 0, 0, nullptr,
            "^([0-9A-Fa-f]{2}:){5}[0-9A-Fa-f]{2}$" } } },
    { E("🆔"), "STA MAC (DEF)", "AT+CIPSTAMAC_DEF=\"%1\"", AtStandard::Hayes, "AT+CIPSTAMAC_DEF=\"AA:BB:CC:DD:EE:FF\"", false,
        { { "MAC", true, nullptr, "AA:BB:CC:DD:EE:FF", AtArgType::Regex, 0, 0, nullptr,
            "^([0-9A-Fa-f]{2}:){5}[0-9A-Fa-f]{2}$" } } },
    { E("🆔"), "AP  MAC (CUR)", "AT+CIPAPMAC_CUR=\"%1\"", AtStandard::Hayes, "AT+CIPAPMAC_CUR=\"AA:BB:CC:DD:EE:FF\"", false,
        { { "MAC", true, nullptr, "AA:BB:CC:DD:EE:FF", AtArgType::Regex, 0, 0, nullptr,
            "^([0-9A-Fa-f]{2}:){5}[0-9A-Fa-f]{2}$" } } },
    { E("🆔"), "AP  MAC (DEF)", "AT+CIPAPMAC_DEF=\"%1\"", AtStandard::Hayes, "AT+CIPAPMAC_DEF=\"AA:BB:CC:DD:EE:FF\"", false,
        { { "MAC", true, nullptr, "AA:BB:CC:DD:EE:FF", AtArgType::Regex, 0, 0, nullptr,
            "^([0-9A-Fa-f]{2}:){5}[0-9A-Fa-f]{2}$" } } },

    { E("🌐"), "STA IP (CUR)", "AT+CIPSTA_CUR=\"%1\",\"%2\",\"%3\"", AtStandard::Hayes,
      "AT+CIPSTA_CUR=\"192.168.1.100\",\"192.168.1.1\",\"255.255.255.0\"", false,
        { { "IP",      true, nullptr, "e.g. 192.168.1.100", AtArgType::String, 0, 0, nullptr, nullptr },
          { "Gateway", true, nullptr, "e.g. 192.168.1.1",   AtArgType::String, 0, 0, nullptr, nullptr },
          { "Netmask", true, nullptr, "e.g. 255.255.255.0", AtArgType::String, 0, 0, nullptr, nullptr } } },
    { E("🌐"), "STA IP (DEF)", "AT+CIPSTA_DEF=\"%1\",\"%2\",\"%3\"", AtStandard::Hayes,
      "AT+CIPSTA_DEF=\"192.168.1.100\",\"192.168.1.1\",\"255.255.255.0\"", false,
        { { "IP",      true, nullptr, "e.g. 192.168.1.100", AtArgType::String, 0, 0, nullptr, nullptr },
          { "Gateway", true, nullptr, "e.g. 192.168.1.1",   AtArgType::String, 0, 0, nullptr, nullptr },
          { "Netmask", true, nullptr, "e.g. 255.255.255.0", AtArgType::String, 0, 0, nullptr, nullptr } } },

    { E("🏠"), "AP  IP (CUR)", "AT+CIPAP_CUR=\"%1\",\"%2\",\"%3\"", AtStandard::Hayes,
      "AT+CIPAP_CUR=\"192.168.4.1\",\"192.168.4.1\",\"255.255.255.0\"", false,
        { { "IP",      true, "192.168.4.1", nullptr, AtArgType::String, 0, 0, nullptr, nullptr },
          { "Gateway", true, "192.168.4.1", nullptr, AtArgType::String, 0, 0, nullptr, nullptr },
          { "Netmask", true, "255.255.255.0", nullptr, AtArgType::String, 0, 0, nullptr, nullptr } } },
    { E("🏠"), "AP  IP (DEF)", "AT+CIPAP_DEF=\"%1\",\"%2\",\"%3\"", AtStandard::Hayes,
      "AT+CIPAP_DEF=\"192.168.4.1\",\"192.168.4.1\",\"255.255.255.0\"", false,
        { { "IP",      true, "192.168.4.1", nullptr, AtArgType::String, 0, 0, nullptr, nullptr },
          { "Gateway", true, "192.168.4.1", nullptr, AtArgType::String, 0, 0, nullptr, nullptr },
          { "Netmask", true, "255.255.255.0", nullptr, AtArgType::String, 0, 0, nullptr, nullptr } } },

    { E("🧠"), "SmartConfig start", "AT+CWSTARTSMART=%1", AtStandard::Hayes, "AT+CWSTARTSMART=1", false,
        { { "Type", true, "1", "0=ESP-TOUCH,1=AIRKISS,3=BOTH", AtArgType::Choice, 0, 0, "0|1|3", nullptr } } },
    { E("🧠"), "SmartConfig stop",  "AT+CWSTOPSMART", AtStandard::Hayes, "AT+CWSTOPSMART", false, {} },

    { E("ℹ️ "), "Conn status", "AT+CIPSTATUS", AtStandard::Hayes, "AT+CIPSTATUS", false, {} },
    { E("✉️ "), "Send (CIPSENDEX)", "AT+CIPSENDEX=%1", AtStandard::Hayes, "AT+CIPSENDEX=12", false,
        { { "Length", true, "12", nullptr, AtArgType::Uint, 1, 2048, nullptr, nullptr } } },
    { E("📦"),  "Send to buffer", "AT+CIPSENDBUF=%1", AtStandard::Hayes, "AT+CIPSENDBUF=64", false,
        { { "Length", true, "64", nullptr, AtArgType::Uint, 1, 4096, nullptr, nullptr } } },
    { E("♻️ "), "Reset seg IDs", "AT+CIPBUFRESET", AtStandard::Hayes, "AT+CIPBUFRESET", false, {} },
    { E("📊"),  "Buffer status", "AT+CIPBUFSTATUS", AtStandard::Hayes, "AT+CIPBUFSTATUS", false, {} },
    { E("🔎"),  "Check segment", "AT+CIPCHECKSEQ=%1", AtStandard::Hayes, "AT+CIPCHECKSEQ=1", false,
        { { "SegmentID", true, "1", nullptr, AtArgType::Uint, 0, 65535, nullptr, nullptr } } },

    { E("🔀"), "Multi-conn (CIPMUX)", "AT+CIPMUX=%1", AtStandard::Hayes, "AT+CIPMUX=1", false,
        { { "Enable", true, "1", "0=single,1=multi", AtArgType::Bool01, 0, 0, nullptr, nullptr } } },
    { E("🖥️ "), "Server mode", "AT+CIPSERVER=%1,%2", AtStandard::Hayes, "AT+CIPSERVER=1,8080", false,
        { { "Mode", true, "1", "0=stop,1=start", AtArgType::Bool01, 0, 0, nullptr, nullptr },
          { "Port", true, "8080", nullptr, AtArgType::Uint, 1, 65535, nullptr, nullptr } } },
    { E("🔁"), "Transparent mode", "AT+CIPMODE=%1", AtStandard::Hayes, "AT+CIPMODE=1", false,
        { { "Mode", true, "1", "0=normal,1=transparent", AtArgType::Bool01, 0, 0, nullptr, nullptr } } },
    { E("💾"), "Save trans link", "AT+SAVETRANSLINK=%1,\"%2\",%3", AtStandard::Hayes,
      "AT+SAVETRANSLINK=1,\"example.com\",80", false,
        { { "Enable", true, "1", "0/1", AtArgType::Bool01, 0, 0, nullptr, nullptr },
          { "Host",   true, "example.com", nullptr, AtArgType::String, 0, 0, nullptr, nullptr },
          { "Port",   true, "80", nullptr, AtArgType::Uint, 1, 65535, nullptr, nullptr } } },
    { E("⏱️ "), "Server timeout", "AT+CIPSTO=%1", AtStandard::Hayes, "AT+CIPSTO=300", false,
        { { "Seconds", true, "300", nullptr, AtArgType::Uint, 0, 7200, nullptr, nullptr } } },
    { E("⬆️ "), "Firmware update (net)", "AT+CIUPDATE", AtStandard::Hayes, "AT+CIUPDATE", true, {} },
    { E("🏓"), "Ping host", "AT+PING=\"%1\"", AtStandard::Hayes, "AT+PING=\"example.com\"", false,
        { { "Host", true, "example.com", nullptr, AtArgType::String, 0, 0, nullptr, nullptr } } },
    { E("🔁"),"Open TCP","AT+CIPSTART=\"TCP\",\"%1\",%2", AtStandard::SIMCom,
      "AT+CIPSTART=\"TCP\",\"example.com\",80", false,
        { { "Host", true, nullptr, "example.com", AtArgType::String, 0, 0, nullptr, nullptr },
          { "Port", true, "80", nullptr, AtArgType::Uint, 1, 65535, nullptr, nullptr } } },
    { E("✉️ "),"Send (enter data)","AT+CIPSEND", AtStandard::SIMCom, "AT+CIPSEND", false, {} },
    { E("🧹"),"Close","AT+CIPCLOSE", AtStandard::SIMCom, "AT+CIPCLOSE", false, {} },
    { E("🧯"),"Shutdown stack","AT+CIPSHUT", AtStandard::SIMCom, "AT+CIPSHUT", false, {} },
    { E("⚙️ "),"Define PDP (QI)","AT+QICSGP=%1,1,\"%2\",\"\",\"\",1", AtStandard::Quectel,
      "AT+QICSGP=1,1,\"internet\",\"\",\"\",1", false,
        { { "CID", true, "1", nullptr, AtArgType::Uint, 1, 16, nullptr, nullptr },
          { "APN", true, "internet", nullptr, AtArgType::Regex, 0, 0, nullptr, "^[A-Za-z0-9._-]+$" } } },
    { E("▶️ "),"Activate PDP (QI)","AT+QIACT=%1", AtStandard::Quectel, "AT+QIACT=1", false,
        { { "CID", true, "1", nullptr, AtArgType::Uint, 1, 16, nullptr, nullptr } } },
    { E("🔁"),"Open socket","AT+QIOPEN=%1,%2,\"TCP\",\"%3\",%4,0,1", AtStandard::Quectel,
      "AT+QIOPEN=1,0,\"TCP\",\"example.com\",80,0,1", false,
        { { "ContextId", true, "1", nullptr, AtArgType::Uint, 1, 16, nullptr, nullptr },
          { "ConnId",   true, "0", nullptr, AtArgType::Uint, 0, 11, nullptr, nullptr },
          { "Host",     true, nullptr, "example.com", AtArgType::String, 0, 0, nullptr, nullptr },
          { "Port",     true, "80", nullptr, AtArgType::Uint, 1, 65535, nullptr, nullptr } } },
    { E("✉️ "),"Send len","AT+QISEND=%1,%2", AtStandard::Quectel, "AT+QISEND=0,12", false,
        { { "ConnId", true, "0", nullptr, AtArgType::Uint, 0, 11, nullptr, nullptr },
          { "Length", true, "12", nullptr, AtArgType::Uint, 1, 1500, nullptr, nullptr } } },
    { E("📥"),"Read len","AT+QIRD=%1,%2", AtStandard::Quectel, "AT+QIRD=0,512", false,
        { { "ConnId", true, "0", nullptr, AtArgType::Uint, 0, 11, nullptr, nullptr },
          { "Length", true, "512", nullptr, AtArgType::Uint, 1, 1500, nullptr, nullptr } } },
    { E("🧹"),"Close","AT+QICLOSE=%1", AtStandard::Quectel, "AT+QICLOSE=0", false,
        { { "ConnId", true, "0", nullptr, AtArgType::Uint, 0, 11, nullptr, nullptr } } },
};

// HTTP(S)
static const std::vector<AtActionItem> kHttpActions = {
    { E("🚀"),"HTTP init","AT+HTTPINIT", AtStandard::SIMCom, "AT+HTTPINIT", false, {} },
    { E("⚙️ "),"HTTP param URL","AT+HTTPPARA=\"URL\",\"%1\"", AtStandard::SIMCom,
      "AT+HTTPPARA=\"URL\",\"http://example.com/\"", false,
        { { "URL", true, nullptr, "http://...", AtArgType::String, 0, 0, nullptr, nullptr } } },
    { E("▶️ "),"HTTP action","AT+HTTPACTION=%1", AtStandard::SIMCom, "AT+HTTPACTION=0", false,
        { { "Method", true, "0", "0=GET,1=POST,2=HEAD", AtArgType::Choice, 0, 0, "0|1|2", nullptr } } },
    { E("📥"),"HTTP read","AT+HTTPREAD", AtStandard::SIMCom, "AT+HTTPREAD", false, {} },
    { E("🧹"),"HTTP term","AT+HTTPTERM", AtStandard::SIMCom, "AT+HTTPTERM", false, {} },

    { E("🚀"),"HTTP GET","AT+QHTTPGET=\"%1\",%2", AtStandard::Quectel,
      "AT+QHTTPGET=\"http://example.com\",60", false,
        { { "URL", true, nullptr, "http://...", AtArgType::String, 0, 0, nullptr, nullptr },
          { "Timeout (s)", true, "60", nullptr, AtArgType::Uint, 1, 600, nullptr, nullptr } } },
    { E("✉️ "),"HTTP POST","AT+QHTTPPOST=\"%1\",%2,%3,%4", AtStandard::Quectel,
      "AT+QHTTPPOST=\"http://example.com\",9,128,60", false,
        { { "URL", true, nullptr, "http://...", AtArgType::String, 0, 0, nullptr, nullptr },
          { "CT length", true, "9", "e.g. length of 'text/plain'", AtArgType::Uint, 1, 4096, nullptr, nullptr },
          { "Data length", true, "128", nullptr, AtArgType::Uint, 1, 1048576, nullptr, nullptr },
          { "Timeout (s)", true, "60", nullptr, AtArgType::Uint, 1, 600, nullptr, nullptr } } },
    { E("📥"),"HTTP read","AT+QHTTPREAD=%1", AtStandard::Quectel, "AT+QHTTPREAD=60", false,
        { { "Timeout (s)", true, "60", nullptr, AtArgType::Uint, 1, 600, nullptr, nullptr } } },
};

// GNSS
static const std::vector<AtActionItem> kGnssActions = {
    { E("🧭"),"GNSS power","AT+CGNSPWR=%1", AtStandard::SIMCom, "AT+CGNSPWR=1", false,
        { { "Power", true, "1", "0/1", AtArgType::Bool01, 0, 0, nullptr, nullptr } } },
    { E("📡"),"GNSS info","AT+CGNSINF", AtStandard::SIMCom, "AT+CGNSINF", false, {} },
    { E("📍"),"GNSS location","AT+CGNSLOC=%1", AtStandard::SIMCom, "AT+CGNSLOC=2", false,
        { { "Mode", true, "2", "module-specific", AtArgType::Uint, 0, 10, nullptr, nullptr } } },

    { E("🧭"),"GPS on/off","AT+QGPS=%1", AtStandard::Quectel, "AT+QGPS=1", false,
        { { "Power", true, "1", "0/1", AtArgType::Bool01, 0, 0, nullptr, nullptr } } },
    { E("⚙️ "),"GPS config","AT+QGPSCFG=\"%1\",%2", AtStandard::Quectel,
      "AT+QGPSCFG=\"gnssconfig\",3", false,
        { { "Item",  true, "gnssconfig", nullptr, AtArgType::String, 0, 0, nullptr, nullptr },
          { "Value", true, "3", nullptr, AtArgType::Uint, 0, 100, nullptr, nullptr } } },
    { E("📍"),"GPS location","AT+QGPSLOC=%1", AtStandard::Quectel, "AT+QGPSLOC=30", false,
        { { "Timeout (s)", true, "30", nullptr, AtArgType::Uint, 1, 600, nullptr, nullptr } } },
    { E("🧾"),"NMEA output","AT+QGPSGNMEA=\"%1\"", AtStandard::Quectel, "AT+QGPSGNMEA=\"GGA\"", false,
        { { "Sentence", true, "GGA", "GGA/RMC/GSV/...", AtArgType::Choice, 0, 0, "GGA|RMC|GSV|GSA|VTG|GLL", nullptr } } },
};


// LoRa
static const std::vector<AtActionItem> kLoraActions = {
    { E("ℹ️ "), "Module version", "AT+VER", AtStandard::Hayes, "AT+VER", false, {} },
    { E("🌍"), "Set region", "AT+REGION=\"%1\"", AtStandard::Hayes, "AT+REGION=\"EU868\"", false,
        { { "Region", true, "EU868", "EU868/US915/AS923/AU915/KR920/IN865",
            AtArgType::Choice, 0, 0, "EU868|US915|AS923|AU915|KR920|IN865", nullptr } } },
    { E("🅰️ "), "Set DevEUI (OTAA)", "AT+DEVEUI=%1", AtStandard::Hayes, "AT+DEVEUI=0011223344556677", false,
        { { "DevEUI hex", true, nullptr, "16 hex (8B), no spaces",
            AtArgType::Regex, 0, 0, nullptr, "^[0-9A-Fa-f]{16}$" } } },
    { E("🅱️ "), "Set AppEUI (OTAA)", "AT+APPEUI=%1", AtStandard::Hayes, "AT+APPEUI=0102030405060708", false,
        { { "AppEUI hex", true, nullptr, "16 hex (8B), no spaces",
            AtArgType::Regex, 0, 0, nullptr, "^[0-9A-Fa-f]{16}$" } } },
    { E("🔑"), "Set AppKey (OTAA)", "AT+APPKEY=%1", AtStandard::Hayes, "AT+APPKEY=00112233445566778899AABBCCDDEEFF", false,
        { { "AppKey hex", true, nullptr, "32 hex (16B), no spaces",
            AtArgType::Regex, 0, 0, nullptr, "^[0-9A-Fa-f]{32}$" } } },
    { E("🏷️ "), "Set DevAddr (ABP)", "AT+DEVADDR=%1", AtStandard::Hayes, "AT+DEVADDR=26011BDA", false,
        { { "DevAddr hex", true, nullptr, "8 hex (4B), no spaces",
            AtArgType::Regex, 0, 0, nullptr, "^[0-9A-Fa-f]{8}$" } } },
    { E("🗝️ "), "Set NwkSKey (ABP)", "AT+NWKSKEY=%1", AtStandard::Hayes, "AT+NWKSKEY=00112233445566778899AABBCCDDEEFF", false,
        { { "NwkSKey hex", true, nullptr, "32 hex (16B), no spaces",
            AtArgType::Regex, 0, 0, nullptr, "^[0-9A-Fa-f]{32}$" } } },
    { E("🗝️ "), "Set AppSKey (ABP)", "AT+APPSKEY=%1", AtStandard::Hayes, "AT+APPSKEY=00112233445566778899AABBCCDDEEFF", false,
        { { "AppSKey hex", true, nullptr, "32 hex (16B), no spaces",
            AtArgType::Regex, 0, 0, nullptr, "^[0-9A-Fa-f]{32}$" } } },
    { E("🔗"), "Join (OTAA)", "AT+JOIN", AtStandard::Hayes, "AT+JOIN", false, {} },
    { E("🔁"), "ADR on/off", "AT+ADR=%1", AtStandard::Hayes, "AT+ADR=1", false,
        { { "ADR", true, "1", "0/1", AtArgType::Bool01, 0, 0, nullptr, nullptr } } },
    { E("📶"), "Data rate (DR)", "AT+DR=%1", AtStandard::Hayes, "AT+DR=5", false,
        { { "DR index", true, "5", "0..15 (region-dependent)", AtArgType::Uint, 0, 15, nullptr, nullptr } } },
    { E("🏷️ "), "Device class", "AT+CLASS=%1", AtStandard::Hayes, "AT+CLASS=A", false,
        { { "Class", true, "A", "A/B/C", AtArgType::Choice, 0, 0, "A|B|C", nullptr } } },
    { E("✉️ "), "Send (unconfirmed)", "AT+SEND=%1,%2", AtStandard::Hayes, "AT+SEND=1,01 AA 02", false,
        { { "FPort", true, "1", "1..223", AtArgType::Uint, 1, 223, nullptr, nullptr },
          { "Payload (hex bytes)", true, nullptr, "e.g. 01 AA 02", AtArgType::HexBytes, 0, 0, nullptr, nullptr } } },
    { E("✅"), "Send (confirmed)", "AT+CSEND=%1,%2", AtStandard::Hayes, "AT+CSEND=1,01 AA 02", false,
        { { "FPort", true, "1", "1..223", AtArgType::Uint, 1, 223, nullptr, nullptr },
          { "Payload (hex bytes)", true, nullptr, "e.g. 01 AA 02", AtArgType::HexBytes, 0, 0, nullptr, nullptr } } },
    { E("🔍"), "Join status", "AT+NJS?", AtStandard::Hayes, "AT+NJS?", false, {} },
};


// Audio (3GPP)
static const std::vector<AtActionItem> kAudioActions = {
    { E("🔊"), "Speaker volume", "AT+CLVL=%1", AtStandard::ThreeGPP, "AT+CLVL=5", false,
        { { "Level", true, "5", "0..15 (module-dependent)", AtArgType::Uint, 0, 15, nullptr, nullptr } } },
    { E("🤫"), "Mic mute", "AT+CMUT=%1", AtStandard::ThreeGPP, "AT+CMUT=1", false,
        { { "Mute", true, "1", "0=off,1=on", AtArgType::Bool01, 0, 0, nullptr, nullptr } } },
    { E("🔔"), "Ring volume", "AT+CRSL=%1", AtStandard::ThreeGPP, "AT+CRSL=2", false,
        { { "Level", true, "2", "0..5/7/100 (depends module)", AtArgType::Uint, 0, 100, nullptr, nullptr } } },
    { E("🔈"), "Audio route (SIMCom)", "AT+CHFA=%1", AtStandard::SIMCom, "AT+CHFA=1", false,
        { { "Path", true, "1", "0=handset,1=speaker,2=headset", AtArgType::Choice, 0, 0, "0|1|2", nullptr } } },
};

// Power / PSM (3GPP + 2G vendor)
static const std::vector<AtActionItem> kPowerActions = {
    { E("💤"), "PSM on/off", "AT+CPSMS=%1", AtStandard::ThreeGPP, "AT+CPSMS=1", false,
        { { "Enable", true, "1", "0/1", AtArgType::Bool01, 0, 0, nullptr, nullptr } } },
    { E("⏱️ "), "PSM timers", "AT+CPSMS=1,,,\"%1\",\"%2\"", AtStandard::ThreeGPP,
      "AT+CPSMS=1,,,\"00100001\",\"00000101\"", false,
        { { "TAU (T3412 bin8)",  true, "00100001", "8 bits binaires", AtArgType::Regex, 0, 0, nullptr, "^[01]{8}$" },
          { "ActiveTime (T3324)", true, "00000101", "8 bits binaires", AtArgType::Regex, 0, 0, nullptr, "^[01]{8}$" } } },
    { E("🧩"), "eDRX set", "AT+CEDRXS=%1,%2,\"%3\"", AtStandard::ThreeGPP, "AT+CEDRXS=2,4,\"0101\"", false,
        { { "Mode",  true, "2",  "0=off,2=on", AtArgType::Choice, 0, 0, "0|2", nullptr },
          { "AcT",   true, "4",  "4=LTE-M,5=NB-IoT", AtArgType::Choice, 0, 0, "4|5", nullptr },
          { "eDRX",  true, "0101", "4 bits binaires", AtArgType::Regex, 0, 0, nullptr, "^[01]{4}$" } } },
    { E("🔎"), "eDRX readback", "AT+CEDRXRDP", AtStandard::ThreeGPP, "AT+CEDRXRDP", false, {} },
    { E("🌙"), "2G sleep (SIMCom)", "AT+CSCLK=%1", AtStandard::SIMCom, "AT+CSCLK=1", false,
        { { "Sleep", true, "1", "0/1", AtArgType::Bool01, 0, 0, nullptr, nullptr } } },
};


// Hardware I/O & Status
static const std::vector<AtActionItem> kHardwareActions = {
    { E("🔋"), "Battery status", "AT+CBC", AtStandard::ThreeGPP, "AT+CBC", false, {} },
    { E("💳"), "SIM ICCID", "AT+CCID", AtStandard::ThreeGPP, "AT+CCID", false, {} },
    { E("🧾"), "IMSI", "AT+CIMI", AtStandard::ThreeGPP, "AT+CIMI", false, {} },
    { E("📈"), "ADC read (SIMCom)", "AT+CADC?", AtStandard::SIMCom, "AT+CADC?", false, {} },
    { E("📇"), "Subscriber number", "AT+CNUM", AtStandard::ThreeGPP, "AT+CNUM", false, {} },
};

// MQTT (Quectel + SIMCom)
static const std::vector<AtActionItem> kMqttActions = {
    // Quectel QMT*
    { E("🔌"), "QMT open", "AT+QMTOPEN=%1,\"%2\",%3", AtStandard::Quectel,
      "AT+QMTOPEN=0,\"broker.hivemq.com\",1883", false,
        { { "ClientIdx", true, "0", nullptr, AtArgType::Uint, 0, 5, nullptr, nullptr },
          { "Host",      true, nullptr, "broker", AtArgType::String, 0, 0, nullptr, nullptr },
          { "Port",      true, "1883", nullptr, AtArgType::Uint, 1, 65535, nullptr, nullptr } } },
    { E("🔐"), "QMT connect", "AT+QMTCONN=%1,\"%2\"", AtStandard::Quectel,
      "AT+QMTCONN=0,\"client123\"", false,
        { { "ClientIdx", true, "0", nullptr, AtArgType::Uint, 0, 5, nullptr, nullptr },
          { "ClientID",  true, "client123", nullptr, AtArgType::String, 0, 0, nullptr, nullptr } } },
    { E("📨"), "QMT publish", "AT+QMTPUB=%1,%2,%3,%4,\"%5\"", AtStandard::Quectel,
      "AT+QMTPUB=0,1,0,0,\"test/topic\"", false,
        { { "ClientIdx", true, "0", nullptr, AtArgType::Uint, 0, 5, nullptr, nullptr },
          { "MsgId",     true, "1", nullptr, AtArgType::Uint, 1, 65535, nullptr, nullptr },
          { "QoS",       true, "0", "0/1/2", AtArgType::Choice, 0, 0, "0|1|2", nullptr },
          { "Retain",    true, "0", "0/1",   AtArgType::Bool01, 0, 0, nullptr, nullptr },
          { "Topic",     true, "test/topic", nullptr, AtArgType::String, 0, 0, nullptr, nullptr } } },
    { E("📥"), "QMT subscribe", "AT+QMTSUB=%1,%2,\"%3\",%4", AtStandard::Quectel,
      "AT+QMTSUB=0,2,\"test/topic\",0", false,
        { { "ClientIdx", true, "0", nullptr, AtArgType::Uint, 0, 5, nullptr, nullptr },
          { "MsgId",     true, "2", nullptr, AtArgType::Uint, 1, 65535, nullptr, nullptr },
          { "Topic",     true, "test/topic", nullptr, AtArgType::String, 0, 0, nullptr, nullptr },
          { "QoS",       true, "0", "0/1/2", AtArgType::Choice, 0, 0, "0|1|2", nullptr } } },
    { E("🧹"), "QMT disconnect", "AT+QMTDISC=%1", AtStandard::Quectel, "AT+QMTDISC=0", false,
        { { "ClientIdx", true, "0", nullptr, AtArgType::Uint, 0, 5, nullptr, nullptr } } },

    // SIMCom SM*
    { E("🔗"), "SM connect", "AT+SMCONN", AtStandard::SIMCom, "AT+SMCONN", false, {} },
    { E("📨"), "SM publish", "AT+SMPUB=\"%1\",%2,%3,%4", AtStandard::SIMCom,
      "AT+SMPUB=\"test/topic\",5,0,0", false,
        { { "Topic",  true, "test/topic", nullptr, AtArgType::String, 0, 0, nullptr, nullptr },
          { "Length", true, "5", nullptr, AtArgType::Uint, 1, 1048576, nullptr, nullptr },
          { "QoS",    true, "0", "0/1/2", AtArgType::Choice, 0, 0, "0|1|2", nullptr },
          { "Retain", true, "0", "0/1",   AtArgType::Bool01, 0, 0, nullptr, nullptr } } },
    { E("🧹"), "SM disconnect", "AT+SMDISC", AtStandard::SIMCom, "AT+SMDISC", false, {} },
};

// File Storage (Quectel + SIMCom)
static const std::vector<AtActionItem> kFsActions = {
    // Quectel QF*
    { E("📂"), "List files (Quectel)", "AT+QFLST", AtStandard::Quectel, "AT+QFLST", false, {} },
    { E("⬆️ "), "Upload UART (Quectel)", "AT+QFUPL=\"%1\",%2", AtStandard::Quectel,
      "AT+QFUPL=\"/dat/cert.pem\",1024", false,
        { { "Path",  true, "/dat/file.bin", nullptr, AtArgType::String, 0, 0, nullptr, nullptr },
          { "Bytes", true, "1024", nullptr, AtArgType::Uint, 1, 10485760, nullptr, nullptr } } },
    { E("⬇️ "), "Download UART (Quectel)", "AT+QFDWL=\"%1\"", AtStandard::Quectel,
      "AT+QFDWL=\"/dat/file.bin\"", false,
        { { "Path",  true, "/dat/file.bin", nullptr, AtArgType::String, 0, 0, nullptr, nullptr } } },
    { E("🗑️ "), "Delete file (Quectel)", "AT+QFDEL=\"%1\"", AtStandard::Quectel,
      "AT+QFDEL=\"/dat/file.bin\"", false,
        { { "Path",  true, "/dat/file.bin", nullptr, AtArgType::String, 0, 0, nullptr, nullptr } } },

    // SIMCom CFS*
    { E("✍️ "), "Write file (SIMCom)", "AT+CFSWFILE=\"%1\",%2,%3,%4", AtStandard::SIMCom,
      "AT+CFSWFILE=\"/c/cert.pem\",0,512,10", false,
        { { "Path",   true, "/c/file.bin", nullptr, AtArgType::String, 0, 0, nullptr, nullptr },
          { "Offset", true, "0", nullptr, AtArgType::Uint, 0, 10485760, nullptr, nullptr },
          { "Length", true, "512", nullptr, AtArgType::Uint, 1, 10485760, nullptr, nullptr },
          { "Mode",   true, "10", "vendor-defined", AtArgType::Uint, 0, 255, nullptr, nullptr } } },
    { E("📖"), "Read file (SIMCom)", "AT+CFSRFILE=\"%1\",%2,%3", AtStandard::SIMCom,
      "AT+CFSRFILE=\"/c/file.bin\",0,512", false,
        { { "Path",   true, "/c/file.bin", nullptr, AtArgType::String, 0, 0, nullptr, nullptr },
          { "Offset", true, "0", nullptr, AtArgType::Uint, 0, 10485760, nullptr, nullptr },
          { "Length", true, "512", nullptr, AtArgType::Uint, 1, 10485760, nullptr, nullptr } } },
};

// ================== Selector ==================

inline const std::vector<AtActionItem>& getAtActionsFor(AtMode m) {
    switch (m) {
        case AtMode::Config:        return kConfigActions;
        case AtMode::Info:          return kInfoActions;
        case AtMode::SimPin:        return kSimPinActions;
        case AtMode::Network:       return kNetworkActions;
        case AtMode::Calls:         return kCallsActions;
        case AtMode::Sms:           return kSmsActions;
        case AtMode::Phonebook:     return kPhonebookActions;
        case AtMode::Ussd:          return kUssdActions;
        case AtMode::Data:          return kDataActions;
        case AtMode::TimeClock:     return kTimeClockActions;
        case AtMode::TcpIp_Vendor:  return kTcpIpActions;
        case AtMode::Http_Vendor:   return kHttpActions;
        case AtMode::Gnss_Vendor:   return kGnssActions;
        case AtMode::Lora:          return kLoraActions;
        case AtMode::Audio:         return kAudioActions;
        case AtMode::Power:         return kPowerActions;
        case AtMode::Hardware:      return kHardwareActions;
        case AtMode::Mqtt:          return kMqttActions;
        case AtMode::Files:         return kFsActions;
    }
    return kConfigActions; // fallback
}
