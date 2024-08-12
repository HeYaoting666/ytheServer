#pragma once
#include <string>
#include <google/protobuf/service.h>
#include <google/protobuf/stubs/callback.h>
#include "../tcp/ip_net_addr.h"

namespace ythe {

class RpcController: public google::protobuf::RpcController {
private:
    int32_t       mErrorCode  = 0;
    std::string   mErrorInfo  = "";
    std::string   mMsgId      = "";

    bool          mIsFailed   = false;
    bool          mIsCanceled = false;
    bool          mIsFinished = false;

    IPNetAddr::sp mLocalAddr   = nullptr;
    IPNetAddr::sp mPeerAddr    = nullptr;

    int           mTimeout     = 1000; // ms

public:
    RpcController() { DEBUGLOG("%s", "RpcController") }

    ~RpcController() override { INFOLOG("%s", "~RpcController") }

public:
    void          Reset() override;

    bool          Failed() const override;

    std::string   ErrorText() const override;

    void          StartCancel() override;

    void          SetFailed(const std::string& reason) override;

    void          NotifyOnCancel(google::protobuf::Closure* callback) override;

    bool          IsCanceled() const override;

    bool          Finished() const;

    void          SetError(int32_t error_code, const std::string& error_info);

    void          SetMsgId(const std::string& msg_id);

    void          SetFinished(bool value);

    void          SetLocalAddr(const IPNetAddr::sp& addr);

    void          SetPeerAddr(const IPNetAddr::sp& addr);

    void          SetTimeout(int timeout);

    int32_t       GetErrorCode() const;

    std::string   GetErrorInfo() const;

    std::string   GetMsgId();

    IPNetAddr::sp GetLocalAddr();

    IPNetAddr::sp GetPeerAddr();

    int           GetTimeout() const;
};

}