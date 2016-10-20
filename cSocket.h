#include <iostream>

class cSocket {
    public:
        cSocket();
        cSocket(std::string ip, int port);
        cSocket(char *ip, int port);
        std::string receiveStr();
        void sendStr(std::string message);
        void closeSock();
        int mysock;
    private:
        bool sockopen;
};
