#ifndef MESSAGES_H

#define MESSAGES_H

#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>

#ifdef WIN32
#include <winsock2.h>
#define ERRNO WSAGetLastError()
#define sclose(x) closesocket(x)
#define MSleep(x) Sleep(x)
#define EWOULDBLOCK WSAEWOULDBLOCK
#ifndef EINPROGRESS
#define EINPROGRESS WSAEINPROGRESS
#endif
#define ETIMEDOUT WSAETIMEDOUT
#define EALREADY WSAEALREADY
typedef int socklen_t;
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#define ERRNO errno
#define sclose(x) close(x)
#define MSleep(x) usleep(x * 1000)
#endif

#define MSG_BUFFERSIZE 0x4000

const char *XMLHeader();

class HTTPMessage
{
    private:
        static std::string get_rfc1123_time();
        int GetMsg(int len, std::string &result);
        bool HeaderAvailable();
    protected:
        unsigned short port_;
        std::string ip_;
        std::string servername_;
        int sock_;
        char buffer_[MSG_BUFFERSIZE];
        int offset_;
    public:
        HTTPMessage() : sock_(-1), offset_(0) {}
        ~HTTPMessage() { if (sock_ >= 0) sclose(sock_); }
        enum {MORE_DATA = -2, QUIT = -666, ERR = -555, TIMEOUT = -444, OK = 1};
        bool Connect(const std::string &ip, unsigned short port, int timeout_msec = -1);
        bool AskData(const std::string &path);
        int WaitRawData(int timeout, std::string &result);
        int WaitMsg(int timeout, std::string &result);
        int GetMsg(std::string &result);
        void ServerName(const std::string &name) { servername_ = name; }
        const std::string &ServerName() const { return servername_; }
        int GetBytes(unsigned char *b, int len, int retries = 0);
        static std::string PrepareMsg(const std::string &body, const std::string &type = "text/plain");

        static bool SendMsg(int socket, const std::string &msg, const std::string &type = "text/plain");
        bool SendMsg(const std::string &body, const std::string &type = "text/plain") { return SendMsg(sock_, body, type); }
        bool SendXML(const std::string &body);
        static std::string PrepareXML(const std::string &body);
        bool StripHeader();

        int Sock() { return sock_; }
        void Disconnect() { if (sock_ >= 0) sclose(sock_); sock_ = -1; }

};

#endif
