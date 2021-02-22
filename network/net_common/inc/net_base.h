//
// Created by 张荣晋 on 2021/2/21.
//

#ifndef NET_COMMON_NET_BASE_H
#define NET_COMMON_NET_BASE_H

namespace cpplab {
    namespace net {

        template<typename T>
        class BaseInterface {
        public:
            virtual ~BaseInterface() {}
        };

    }
}

#endif //NET_COMMON_NET_BASE_H
