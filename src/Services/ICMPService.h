#pragma once

class ICMPService {
public:
    ICMPService();

    void startTask(int verbosity);
    static void scanTask(void *pvParams);
private:

};
