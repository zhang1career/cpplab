//
// Created by 张荣晋 on 2021/2/20.
//

#ifndef NET_COMMON_NET_CONNECTION_H
#define NET_COMMON_NET_CONNECTION_H

#include "net_common.h"
#include "net_base.h"
#include "net_message.h"
#include "net_tsqueue.h"

namespace cpplab {
    namespace net {

        template<typename T>
        class ServerInterface;

        template<typename T>
        class Connection : public std::enable_shared_from_this<Connection<T>> {
        public:
            enum class Owner {
                Server,
                Client
            };

        public:
            Connection(
                    Owner parent,
                    std::shared_ptr<BaseInterface<T>> baseInterface,
                    asio::io_context &asioContext,
                    asio::ip::tcp::socket socket,
                    Tsqueue<OwnedMessage<T>>& qMessagesIn)
                    : m_asioContext(asioContext),
                      m_socket(std::move(socket)),
                      m_qMessagesIn(qMessagesIn) {
                m_nOwnerType = parent;
                m_baseInterface = baseInterface;

                if (m_nOwnerType == Owner::Server) {
                    m_nHandshakeOut = uint64_t(std::chrono::system_clock::now().time_since_epoch().count());
                    m_nHandshakeCheck = scramble(m_nHandshakeOut);
                } else {
                    m_nHandshakeIn = 0;
                    m_nHandshakeOut = 0;
                }
            }

            virtual ~Connection() {}

        public:
            uint32_t getId() const {
                return m_nId;
            }

            void connectToServer(const asio::ip::tcp::resolver::results_type& endpoints) {
                if (m_nOwnerType == Owner::Client) {
                    asio::async_connect(
                            m_socket,
                            endpoints,
                            [this](std::error_code ec, asio::ip::tcp::endpoint endpoint) {
                                if (!ec) {
                                    readValidation();
                                }
                            });
                }
            }

            void connectToClient(uint32_t uid = 0) {
                if (m_nOwnerType == Owner::Server) {
                    if (m_socket.is_open()) {
                        m_nId = uid;
                        writeValidation();
                        readValidation(std::static_pointer_cast<ServerInterface<T>>(m_baseInterface));
                    }
                }
            }

            void disconnect() {
                if (isConnected()) {
                    asio::post(m_asioContext, [this]() { m_socket.close(); });
                }
            }

            bool isConnected() const {
                return m_socket.is_open();
            }

            void startListening() {}

            void send(const Message<T> &msg) {
                asio::post(
                        m_asioContext,
                        [this, msg]() {
                            bool bWritingMessage = !m_qMessagesOut.empty();
                            m_qMessagesOut.pushBack(msg);
                            if (!bWritingMessage) {
                                writeHeader();
                            }
                        });
            }

        private:
            void writeHeader() {
                asio::async_write(
                        m_socket,
                        asio::buffer(&m_qMessagesOut.front().header, sizeof(MessageHeader<T>)),
                        [this](std::error_code ec, std::size_t length) {
                            if (!ec) {
                                if (m_qMessagesOut.front().body.size() > 0) {
                                    writeBody();
                                } else {
                                    m_qMessagesOut.popFront();
                                    if (!m_qMessagesOut.empty()) {
                                        writeHeader();
                                    }
                                }
                            } else {
                                std::cout << "[" << m_nId << "] Write Header Fail.\n";
                                m_socket.close();
                            }
                        });
            }

            void writeBody() {
                asio::async_write(
                        m_socket,
                        asio::buffer(m_qMessagesOut.front().body.data(), m_qMessagesOut.front().body.size()),
                        [this](std::error_code ec, std::size_t length) {
                            if (!ec) {
                                m_qMessagesOut.popFront();
                                if (!m_qMessagesOut.empty()) {
                                    writeHeader();
                                }
                            } else {
                                std::cout << "[" << m_nId << "] Write Body Fail.\n";
                                m_socket.close();
                            }
                        });
            }

            void readHeader() {
                asio::async_read(
                        m_socket,
                        asio::buffer(&m_msgTemporaryIn.header, sizeof(MessageHeader<T>)),
                        [this](std::error_code ec, std::size_t length) {
                            if (!ec) {
                                if (m_msgTemporaryIn.header.size > 0) {
                                    m_msgTemporaryIn.body.resize(m_msgTemporaryIn.header.size);
                                    readBody();
                                } else {
                                    addToIncomingMessageQueue();
                                }
                            } else {
                                std::cout << "[" << m_nId << "] Read Header Fail.\n";
                                m_socket.close();
                            }
                        });
            }

            void readBody() {
                asio::async_read(
                        m_socket,
                        asio::buffer(m_msgTemporaryIn.body.data(), m_msgTemporaryIn.body.size()),
                        [this](std::error_code ec, std::size_t length) {
                            if (!ec) {
                                addToIncomingMessageQueue();
                            } else {
                                std::cout << "[" << m_nId << "] Read Body Fail.\n";
                                m_socket.close();
                            }
                        });
            }

            void writeValidation() {
                asio::async_write(
                        m_socket,
                        asio::buffer(&m_nHandshakeOut, sizeof(uint64_t)),
                        [this](std::error_code ec, std::size_t length) {
                            if (!ec) {
                                if (m_nOwnerType == Owner::Client) {
                                    readHeader();
                                }
                            } else {
                                m_socket.close();
                            }
                        });
            }

            void readValidation(std::shared_ptr<ServerInterface<T>> server = nullptr) {
                asio::async_read(
                        m_socket,
                        asio::buffer(&m_nHandshakeIn, sizeof(uint64_t)),
                        [this, server](std::error_code ec, std::size_t length) {
                            if (!ec) {
                                if (m_nOwnerType == Owner::Server) {
                                    if (m_nHandshakeIn == m_nHandshakeCheck) {
                                        std::cout << "Client Validated" << std::endl;
                                        server->onClientValidated(this->shared_from_this());
                                        readHeader();
                                    }
                                } else {
                                    m_nHandshakeOut = scramble(m_nHandshakeIn);
                                    writeValidation();
                                }
                            } else {
                                std::cout << "Client Disconnected (readValidation)" << std::endl;
                                m_socket.close();
                            }
                        });
            }

            void addToIncomingMessageQueue() {
                if (m_nOwnerType == Owner::Server) {
                    m_qMessagesIn.pushBack({this->shared_from_this(), m_msgTemporaryIn});
                } else {
                    m_qMessagesIn.pushBack({nullptr, m_msgTemporaryIn});
                }
                readHeader();
            }

            uint64_t scramble (uint64_t nInput) {
                uint64_t out = nInput ^ 0xDEADBEEFC0DECAFE;
                out = (out & 0xF0F0F0F0F0F0F0F0) >> 4 | (out & 0x0F0F0F0F0F0F0F0F) << 4;
                return out ^ 0xC0DEFACE12345678;
            }

        protected:
            asio::ip::tcp::socket m_socket;
            asio::io_context& m_asioContext;
            Tsqueue<Message<T>> m_qMessagesOut;
            Tsqueue<OwnedMessage<T>>& m_qMessagesIn;
            Message<T> m_msgTemporaryIn;

            Owner m_nOwnerType = Owner::Server;
            std::shared_ptr<BaseInterface<T>> m_baseInterface;
            uint32_t m_nId = 0;

            uint64_t m_nHandshakeOut = 0;
            uint64_t m_nHandshakeIn = 0;
            uint64_t m_nHandshakeCheck = 0;
        };

    }
}

#endif //NET_COMMON_NET_CONNECTION_H
