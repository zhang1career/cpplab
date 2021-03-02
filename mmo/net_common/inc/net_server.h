//
// Created by 张荣晋 on 2021/2/20.
//

#ifndef NET_COMMON_NET_SERVER_H
#define NET_COMMON_NET_SERVER_H

#include "net_common.h"
#include "net_connection.h"
#include "net_message.h"
#include "net_tsqueue.h"

namespace cpplab {
    namespace net {

        template<typename T>
        class ServerInterface : public BaseInterface<T>, public std::enable_shared_from_this<ServerInterface<T>> {
        public:
            ServerInterface(uint16_t port) : m_asioAcceptor(m_asioContext,
                                                            asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)) {}

            virtual ~ServerInterface() {
                stop();
            }

        public:
            bool start() {
                try {
                    waitForClientConnection();
                    m_thrContext = std::thread([this]() { m_asioContext.run(); });
                } catch (const std::exception &e) {
                    std::cerr << "[SERVER] Exception: " << e.what() << "\n";
                    return false;
                }
                std::cout << "[SERVER] Started!\n";
                return true;
            }

            void stop() {
                m_asioContext.stop();
                if (m_thrContext.joinable()) {
                    m_thrContext.join();
                }
                std::cout << "[SERVER] Stopped!\n";
            }

            void waitForClientConnection() {
                m_asioAcceptor.async_accept(
                        [this](std::error_code ec, asio::ip::tcp::socket socket) {
                            if (!ec) {
                                std::cout << "[SERVER] New Connection: " << socket.remote_endpoint() << "\n";
                                std::shared_ptr<Connection<T>> newConn =
                                        std::make_shared<Connection<T>>(
                                                Connection<T>::Owner::Server,
                                                this->shared_from_this(),
                                                m_asioContext,
                                                std::move(socket),
                                                m_qMessagesIn);
                                if (onClientConnect(newConn)) {
                                    m_deqConnections.push_back(std::move(newConn));
                                    m_deqConnections.back()->connectToClient(m_nIdCounter++);
                                    std::cout << "[" << m_deqConnections.back()->getId() << "] Connection Approved\n";
                                } else {
                                    std::cout << "[------] Connection Denied!\n";
                                }
                            } else {
                                std::cout << "[SERVER] New Connection Error: " << ec.message() << "\n";
                            }
                            waitForClientConnection();
                        }
                );
            }

            void messageClient(std::shared_ptr<Connection<T>> client, const Message<T> &msg) {
                if (client && client->isConnected()) {
                    client->send(msg);
                } else {
                    onClientDisconnect(client);
                    client.reset();
                    m_deqConnections.erase(
                            std::remove(m_deqConnections.begin(), m_deqConnections.end(), client),
                            m_deqConnections.end());
                }
            }

            void messageAllClients(const Message<T> &msg, std::shared_ptr<Connection<T>> pIgnoreClient = nullptr) {
                bool bInvalidClientExists = false;
                for (auto& client : m_deqConnections) {
                    if (client && client->isConnected()) {
                        if (client != pIgnoreClient) {
                            client->send(msg);
                        }
                    } else {
                        onClientDisconnect(client);
                        client.reset();
                        bInvalidClientExists = true;
                    }
                }

                if (bInvalidClientExists) {
                    m_deqConnections.erase(
                            std::remove(m_deqConnections.begin(), m_deqConnections.end(), nullptr),
                            m_deqConnections.end());
                }
            }

            void update(size_t nMaxMessages = -1, bool bWait = false) {
                if (bWait) {
                    m_qMessagesIn.wait();
                }
                size_t nMessageCount = 0;
                while (nMessageCount < nMaxMessages && !m_qMessagesIn.empty()) {
                    auto msg = m_qMessagesIn.popFront();
                    onMessage(msg.remote, msg.msg);
                    nMessageCount++;
                }
            }

        public:
            virtual void onClientValidated(std::shared_ptr<Connection<T>> client) {}
        protected:
            virtual bool onClientConnect(std::shared_ptr<Connection<T>> client) {return false;}
            virtual void onClientDisconnect(std::shared_ptr<Connection<T>> client) {}
            virtual void onMessage(std::shared_ptr<Connection<T>> client, Message<T> &msg) {}

        protected:
            Tsqueue<OwnedMessage<T>> m_qMessagesIn;
            std::deque<std::shared_ptr<Connection<T>>> m_deqConnections;
            asio::io_context m_asioContext;
            std::thread m_thrContext;
            asio::ip::tcp::acceptor m_asioAcceptor;
            uint32_t m_nIdCounter = 10000;
        };

    }
}

#endif //NET_COMMON_NET_SERVER_H
