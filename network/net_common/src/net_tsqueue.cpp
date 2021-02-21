//
// Created by 张荣晋 on 2021/2/19.
//

#include "net_tsqueue.h"

namespace olc {
    namespace net {
        template<typename T>
        class Tsqueue {
        public:
            Tsqueue() = default;

            Tsqueue(const Tsqueue<T> &) = delete;

            virtual ~Tsqueue() {
                clear();
            }

        public:
            const T &front() {
                std::scoped_lock lock(muxQueue);
                return deqQueue.front();
            }

            const T &back() {
                std::scoped_lock lock(muxQueue);
                return deqQueue.back();
            }

            T popFront() {
                std::scoped_lock lock(muxQueue);
                auto t = std::move(deqQueue.front());
                deqQueue.pop_front();
                return t;
            }

            T popBack() {
                std::scoped_lock lock(muxQueue);
                auto t = std::move(deqQueue.back());
                deqQueue.pop_back();
                return t;
            }

            void pushFront(const T &item) {
                std::scoped_lock lock(muxQueue);
                deqQueue.emplace_front(std::move(item));
            }

            bool empty() {
                std::scoped_lock lock(muxQueue);
                return deqQueue.empty();
            }

            size_t count() {
                std::scoped_lock lock(muxQueue);
                return deqQueue.size();
            }

            void clear() {
                std::scoped_lock lock(muxQueue);
                deqQueue.clear();
            }

            void wait() {
                while (empty()) {
                    std::unique_lock <std::mutex> ul(muxBlocking);
                    cvBlocking.wait(ul);
                }
            }

        protected:
            std::mutex muxQueue;
            std::deque <T> deqQueue;
            std::condition_variable cvBlocking;
            std::mutex muxBlocking;
        };
    }
}
