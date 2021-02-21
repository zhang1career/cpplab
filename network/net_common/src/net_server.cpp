//
// Created by 张荣晋 on 2021/2/20.
//

#include "net_server.h"

namespace olc {
    namespace net {
        template<typename T>
        class ServerInterface {
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
                                std::shared_ptr <Connection<T>> newConn =
                                        std::make_shared < Connection < T >> (
                                                Connection<T>::owner::server,
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

            void messageClient(std::shared_ptr <Connection<T>> client, const Message <T> &msg) {
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

            void messageAllClients(const Message <T> &msg, std::shared_ptr <Connection<T>> pIgnoreClient = nullptr) {
                bool bInvalidClientExists = false;
                for (auto &client : m_deqConnections) {
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

        protected:
            virtual bool onClientConnect(std::shared_ptr <Connection<T>> client) {
                return false;
            }

            virtual void onClientDisconnect(std::shared_ptr <Connection<T>> client) {

            }

            virtual void onMessage(std::shared_ptr <Connection<T>> client, Message <T> &msg) {

            }

        protected:
            // thread safe queue for incoming message packets
            Tsqueue <OwnedMessage<T>> m_qMessagesIn;
            // container for active validated connection
            std::deque <std::shared_ptr<Connection < T>>>
            m_deqConnections;
            // order of declaration is important - it is also the order of initialisation
            asio::io_context m_asioContext;
            std::thread m_thrContext;
            // these things need an asio context
            asio::ip::tcp::acceptor m_asioAcceptor;
            // clients will be identified in the "wider system" via an ID
            uint32_t m_nIdCounter = 10000;
        };
    }
}
