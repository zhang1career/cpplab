//
// Created by 张荣晋 on 2021/2/20.
//

#include "net_connection.h"

namespace olc {
    namespace net {
        template<typename T>
        class Connection : public std::enable_shared_from_this<Connection<T>> {
        public:
            enum class Owner {
                server,
                client
            };

        public:
            Connection(
                    Owner parent,
                    asio::io_context &asioContext,
                    asio::ip::tcp::socket socket,
                    Tsqueue <OwnedMessage<T>> &qIn)
                    : m_asioContext(asioContext),
                      m_socket(std::move(socket)),
                      m_qMessagesIn(qIn) {
                m_nOwnerType = parent;
            }

            virtual ~Connection() {}

        public:
            uint32_t getId() const {
                return id;
            }

            void connectToServer(const asio::ip::tcp::resolver::results_type &endpoints) {
                if (m_nOwnerType == Owner::client) {
                    asio::async_connect(
                            m_socket,
                            endpoints,
                            [this](std::error_code ec, asio::ip::tcp::endpoint endpoint) {
                                if (!ec) {
                                    readHeader();
                                }
                            });
                }
            }

            void connectToClient(uint32_t uid = 0) {
                if (m_nOwnerType == Owner::server) {
                    if (m_socket.is_open()) {
                        id = uid;
                        readHeader();
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

            bool send(const Message <T> &msg) {
                asio::post(
                        m_asioContext,
                        [this, msg]() {
                            bool bWritingMessage = !m_qMessagesOut.empty();
                            m_qMessagesOut.push_back(msg);
                            if (!bWritingMessage) {
                                writeHeader();
                            }
                        });
            }

        private:
            void writeHeader() {
                asio::async_write(
                        m_socket,
                        asio::buffer(&m_qMessagesOut.front().header, sizeof(MessageHeader < T > )),
                        [this](std::error_code ec, std::size_t length) {
                            if (!ec) {
                                if (m_qMessagesOut.front().body.size() > 0) {
                                    writeBody();
                                } else {
                                    m_qMessagesOut.pop_front();
                                    if (!m_qMessagesOut.empty()) {
                                        writeHeader();
                                    }
                                }
                            } else {
                                std::cout << "[" << id << "] Write Header Fail.\n";
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
                                m_qMessagesOut.pop_front();
                                if (!m_qMessagesOut.empty()) {
                                    writeHeader();
                                }
                            } else {
                                std::cout << "[" << id << "] Write Body Fail.\n";
                                m_socket.close();
                            }
                        });
            }

            void readHeader() {
                asio::async_read(
                        m_socket,
                        asio::buffer(&m_msgTemporaryIn.header, sizeof(MessageHeader < T > )),
                        [this](std::error_code ec, std::size_t length) {
                            if (!ec) {
                                if (m_msgTemporaryIn.header.size > 0) {
                                    m_msgTemporaryIn.body.resize(m_msgTemporaryIn.header.size);
                                    readBody();
                                } else {
                                    addToIncomingMessageQueue();
                                }
                            } else {
                                std::cout << "[" << id << "] Read Header Fail.\n";
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
                                std::cout << "[" << id << "] Read Body Fail.\n";
                                m_socket.close();
                            }
                        });
            }

            void addToIncomingMessageQueue() {
                if (m_nOwnerType == Owner::server) {
                    m_qMessagesIn.push_back({this->shared_from_this(), m_msgTemporaryIn});
                } else {
                    m_qMessagesIn.push_back({nullptr, m_msgTemporaryIn});
                }
                readHeader();
            }

        protected:
            asio::ip::tcp::socket m_socket;
            asio::io_context &m_asioContext;
            Tsqueue <Message<T>> m_qMessagesOut;
            Tsqueue <OwnedMessage<T>> &m_qMessagesIn;
            Message <T> m_msgTemporaryIn;
            Owner m_nOwnerType = Owner::server;
            uint32_t id = 0;
        };
    }
}
