#pragma once

#include <string>

class NetcatService {
public:
    void startTask(const std::string& host,int verbosity, int port);
    
    bool isConnected() const;
    void writeChar(char c);
    std::string readOutput();
    std::string readOutputNonBlocking();
    void close();

private:
    // Netcat Task, cause overflow if it runs in the main loop, so it must run in a dedicated FreeRTOS task with a larger stack
    static void connectTask(void* pvParams);

    bool connect(const std::string& host, int verbosity, int port);
    bool authenticate(const std::string& password);
    bool openChannel();
    bool requestPty();
    bool startShell();

    //ssh_session session = nullptr;
    //ssh_channel channel = nullptr;
    bool connected = false;
};
