#ifndef EC_SOCKET_H
#define EC_SOCKET_H

#include <arpa/inet.h>
#include <future>
#include <netdb.h>
#include <netinet/in.h>
#include <net/if.h>
#include <unistd.h>
#include <vector>
#include <queue>

#include "SocketException.h"
#include "SocketEnum.h"

#define TCP_INTERFACE       "eth0"
#define SOCKET_BUFFER       3200
#define SOCKET_ERROR        (-1)
#define DEFAULT_THREAD      3
#define DEFAULT_LISTEN_Q    5
#define DEFAULT_RECV_TIME   1

namespace CameraKiosk
{
    namespace Util
    {   
        class SocketHandler;

        using uptrSocketHandler = std::unique_ptr<SocketHandler>;
        using SocketInfo_t = std::pair<int, sockaddr_in>;
        using Message_t = std::pair<int, std::stringstream>;

        struct SocketMessage_t
        {
            std::string header;
            std::vector<char> data;
        };

        class SocketAcceptor
        {
        private:
            int _listener;
            bool _canceller = false;

        public:
            SocketAcceptor() {};
            ~SocketAcceptor()
            {
                if (_listener > 0)
                    close(_listener);
            };

            void initSocket();
            void cancelAcceptClient();
            std::future<SocketInfo_t> acceptClientAsync();
            SocketInfo_t acceptClient();
        };

        class SocketCommunicator
        {
            std::queue<SocketMessage_t> sendMessageQ;

        private:
            std::mutex _sendQM;
            int _clientSd;
            bool _canceller = false;

            void recvMessageHandle(char *recvMsg);

        public:
            SocketCommunicator(int clientSd) : _clientSd(clientSd){};
            ~SocketCommunicator()
            {
                if (_clientSd > 0)
                    close(_clientSd);
            };

            ESocketStatus runCommunication();
            void cancelCommunication();
            bool enqueueData(SocketMessage_t message);
            bool sendData(const void *buff, size_t buffSize);
            bool sendData(SocketMessage_t &message);
        };

        class SocketHandler
        {
        private:
            SocketAcceptor *_acceptor = nullptr;
            SocketCommunicator *_communicator = nullptr;

            std::unique_ptr<SocketInfo_t> _acceptClient = nullptr;
            ESocketStatus _status = ESocketStatus::IDLE;

            int handle();

        public:
            SocketHandler() { _acceptor = new SocketAcceptor(); };
            SocketHandler(SocketAcceptor *acceptor) : _acceptor(acceptor){};
            ~SocketHandler() {};

            int openAcceptor();
            std::future<int> runAsyncSocketService();
            void exitService();
            bool sendReqeust(std::string msg, ESocketMessageType type);
            bool sendReqeust(char **binaryData, size_t size, std::string fileName);

            ESocketStatus getStatus();
        };

        void setHeader(SocketMessage_t &message, ESocketMessageType type);
        void setHeader(SocketMessage_t &message, std::string headerName, std::string headerValue);

        extern uptrSocketHandler gSocketHandler;
    };
}

#endif