#pragma once
#include <functional>
#include <google/protobuf/stubs/callback.h>
#include "src/log/logger.h"

namespace ythe {

class RpcClosure : public google::protobuf::Closure {
private:
    std::function<void()> mCb;

public:
    explicit RpcClosure(const std::function<void()>& cb): mCb(cb) { }

    ~RpcClosure() override = default;

public:
    void Run() override {
        if(mCb != nullptr)
            mCb();
    }
};

}