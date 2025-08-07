#pragma once

#include <string>

class NmapService {
public:
    void startTask(const std::string& host, int verbosity);    

private:
    // Nmap Task, cause overflow if it runs in the main loop, so it must run in a dedicated FreeRTOS task with a larger stack
    static void connectTask(void* pvParams);

};
