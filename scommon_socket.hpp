#ifndef _SCOMMON_SOCKET_HPP_
#define _SCOMMON_SOCKET_HPP_
#if defined(__GNUC__) || defined (linux)
#include <netinet/in.h>
#include <arpa/inet.h>
#elif defined(_MSC_VER)
#endif
#include <memory.h>
#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <chrono>
#include <queue>
#include <condition_variable>

#define DEFAULT_IP ("127.0.0.1")
#define DEFAULT_PORT (16669)
#define DEFAULT_BUFLEN (10240)
#define DEFAULT_START (true)
#define DEFAULT_SLEEP_SEC (30)

template <class MsgType>
class UdpClient
{
public:
    UdpClient(const std::string& ip = DEFAULT_IP, int port = DEFAULT_PORT, int bufLen = DEFAULT_BUFLEN, 
        bool start = DEFAULT_START);
    ~UdpClient();
    int Start();
    int SendBlock(const MsgType& msg);
    int Send(const MsgType& msg);
    void Stop();
private:
    std::string ip;
    int port;
    struct sockaddr_in addrServ;
    int fd;
    int len;
    std::atomic<bool> run;
    std::thread thread;
    std::recursive_mutex mtx;
    std::condition_variable_any condVar;
    std::queue<MsgType> msgs;
    unsigned int bufLen;
    unsigned char* sendBuf;
    void Thread();
};

template<class MsgType>
UdpClient<MsgType>::UdpClient(const std::string& ip, int port, int bufLen, bool start):
    ip(ip),
    port(port),
    bufLen(bufLen),
    sendBuf(new unsigned char[bufLen]),
    run(false)
{
    if (start)
        Start();
};

template<class MsgType>
UdpClient<MsgType>::~UdpClient()
{
    delete[] sendBuf;
    Stop();
};

template<class MsgType>
void UdpClient<MsgType>::Thread()
{
    std::unique_lock<std::recursive_mutex> lck(mtx);
    while (run)
    {
        bool ready = condVar.wait_for(lck, std::chrono::seconds(DEFAULT_SLEEP_SEC), [&]() {
            return (!msgs.empty() || !run);
            });
        if (!ready || !run)
            continue;
        SendBlock(msgs.front());
        msgs.pop();
    }
}

template<class MsgType>
int UdpClient<MsgType>::SendBlock(const MsgType& msg)
{
    std::lock_guard<std::recursive_mutex> lck(mtx);
    int size = sizeof(msg);
    memcpy(sendBuf, &msg, size);
    return sendto(fd, sendBuf, size, 0, (struct sockaddr*)&addrServ, len);
}

template<class MsgType>
int UdpClient<MsgType>::Send(const MsgType& msg)
{
    std::lock_guard<std::recursive_mutex> lck(mtx);
    msgs.push(msg);
    condVar.notify_one();
    return 0;
}

template<class MsgType>
int UdpClient<MsgType>::Start()
{
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0)
        return -1;
    addrServ.sin_family = AF_INET;
    addrServ.sin_addr.s_addr = inet_addr(ip.c_str());
    addrServ.sin_port = htons(port);
    len = sizeof(addrServ);
    run = true;
    thread = std::thread(&UdpClient::Thread, this);
    return 0;
}

template<class MsgType>
void UdpClient<MsgType>::Stop()
{
    run = false;
    condVar.notify_all();
    if (thread.joinable())
        thread.join();
}

#undef DEFAULT_IP
#undef DEFAULT_PORT
#undef DEFAULT_BUFLEN
#undef DEFAULT_START
#undef DEFAULT_PERIOD
#endif
