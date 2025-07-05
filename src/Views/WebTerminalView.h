#pragma once
#include "ITerminalView.h"
#include <string>
#include "Servers/WebSocketServer.h"
#include "Enums/TerminalTypeEnum.h"

class WebTerminalView : public ITerminalView {
public:
    WebTerminalView(WebSocketServer &server);

    void initialize() override;
    void welcome(TerminalTypeEnum& terminalType, std::string& terminalInfos) override;
    void print(const std::string& text) override;
    void println(const std::string& text) override;
    void printPrompt(const std::string& mode) override;
    void clear() override;
private:
    WebSocketServer server;
};
