#pragma once

#include <string>
#include <sstream>
#include "Services/SdService.h"
#include "Interfaces/ITerminalView.h"
#include "Interfaces/IInput.h"
#include "Transformers/ArgTransformer.h"
#include "Managers/CommandHistoryManager.h"

class SdCardManager {
public:
    SdCardManager(SdService& sdService, ITerminalView& view, IInput& input,  ArgTransformer& argTransformer);
    void runShell();

private:
    SdService& sd;
    ITerminalView& terminalView;
    IInput& terminalInput;
    ArgTransformer& argTransformer;
    std::string currentDir;
    CommandHistoryManager commandHistoryManager;

    void executeCommand(const std::string& input);

    // Command handlers
    void cmdLs();
    void cmdCd(std::istringstream& iss);
    void cmdMkdir(std::istringstream& iss);
    void cmdTouch(std::istringstream& iss);
    void cmdRm(std::istringstream& iss);
    void cmdCat(std::istringstream& iss);
    void cmdEcho(std::istringstream& iss);
    void cmdHelp();

    // Input reader
    std::string readLine();

    // Path utils
    std::string normalizePath(const std::string& path);
    std::string resolveRelativePath(const std::string& base, const std::string& arg);
};
