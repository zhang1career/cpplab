//
// Created by 张荣晋 on 2021/2/20.
//

#include "net_client.h"

namespace olc {
    namespace net {
        template<typename T>
        class ClientInterface {
        public:
            bool connect(const std::string &host, const uint16_t port) {
                try {
                    asio::ip::tcp::resolver resolver(m_context);
                    asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(host, std::to_string(port));
                    m_connection = std::make_unique < Connection < T >> (
                            Connection<T>::owner::client,
                                    m_context,
                                    asio::ip::tcp::socket(m_context),
                                    m_qMessagesIn);
                    m_connection->connectToServer(endpoints);
                    m_thrContext = std::thread([this]() { m_context.run(); });
                } catch (const std::exception &e) {
                    std::cerr << "Client Exception: " << e.what() << "\n";
                    return false;
                }
                return true;
            }

            void disconnect() {
                if (isConnected()) {
                    m_connection->disconnect();
                }
                m_context.stop();
                if (m_thrContext.joinable()) {
                    m_thrContext.join();
                }
                m_connection.release();
            }

            bool isConnected() {
                if (m_connection) {
                    return m_connection->isConnected();
                }
                return false;
            }

            void send(const Message <T> &msg) {
                if (isConnected())
                    m_connection->send(msg);
            }

            Tsqueue <OwnedMessage<T>> &incoming() {
                return m_qMessagesIn;
            }

        protected:
            asio::io_context m_context;
            std::thread m_thrContext;
            std::unique_ptr <Connection<T>> m_connection;

        private:
            Tsqueue <OwnedMessage<T>> m_qMessagesIn;
        };
    }
}
