#include "rpc_channel.h"
#include "../coder/tinypb_protocol.h"
#include "../coder/tinypb_coder.h"
#include "../entity/error_code.h"
#include "../net/timer_event.h"
#include <google/protobuf/message.h>

namespace ythe {

RpcChannel::RpcChannel(const IPNetAddr::sp& peerAddr)
{
    mClient = std::make_shared<TCPClient>(peerAddr);
    mClient->TCPConnect();

    mCoder = new TinyPBCoder();
}

RpcChannel::~RpcChannel()
{
    if(mCoder != nullptr) {
        delete mCoder;
        mCoder = nullptr;
    }
}

void RpcChannel::CallMethod(const google::protobuf::MethodDescriptor* method, google::protobuf::RpcController* rpcController, 
    const google::protobuf::Message* reqPb, google::protobuf::Message* respPb, google::protobuf::Closure* done)
{   
    auto reqMessage = std::make_shared<TinyPBProtocol>();

    auto myController = dynamic_cast<RpcController *>(rpcController);
    if(myController == nullptr) {
        ERRORLOG("%s", "failed callmethod, RpcController convert error")
        exit(1);
    }
    // 获取 msg_id
    // 如果 controller 指定了 msgId, 直接使用
    if(myController->GetMsgId().empty()) {
        reqMessage->mMsgId = GetMsgID();
        myController->SetMsgId(reqMessage->mMsgId);
    } else {
        reqMessage->mMsgId = myController->GetMsgId();
    }

    // 获取 methodName
    reqMessage->mMethodName = method->full_name();

    // reqPb 序列化至 mPbData
    if(!reqPb->SerializePartialToString(&(reqMessage->mPbData))) {
        std::string errInfo = "failed to serialize";
        myController->StartCancel();
        myController->SetError(ERROR_FAILED_SERIALIZE, errInfo);
        if(done) {
            done->Run();
        };
        myController->SetFinished(true);
        return;
    }

    // TimerEvent::sp timeEvent = std::make_shared<TimerEvent>(myController->GetTimeout(), false, [myController, done]() mutable {
    //     if (myController->Finished()) {
    //         INFOLOG("%s | rpc is finished", myController->GetMsgId().c_str())
    //         return;
    //     }

    //     myController->StartCancel();
    //     myController->SetError(ERROR_RPC_CALL_TIMEOUT, "rpc call timeout " + std::to_string(myController->GetTimeout()));
    //     if(done) {
    //         done->Run();
    //     };
    //     myController->SetFinished(true);
    // });
    // mClient->AddTimerEvent(timeEvent);
    
    // 准备发送数据
    std::vector<AbstractProtocol::sp> reqMessages;
    reqMessages.push_back(reqMessage);
    auto dataToSend = std::make_shared<TCPBuffer>(128);
    mCoder->Encode(reqMessages, dataToSend);
    mClient->SendData(dataToSend);

    // 接收数据
    std::vector<AbstractProtocol::sp> respMessages;
    TCPBuffer::sp dataToRecv;
    mClient->RecvData(dataToRecv);
    mCoder->Decode(dataToRecv, respMessages);
    for(const auto& respMsg : respMessages) {
        if(reqMessage->mMsgId != respMsg->mMsgId)
            continue;
        auto respMessage = std::dynamic_pointer_cast<TinyPBProtocol>(respMsg);
        if(respMessage->mErrCode != 0) {
            myController->StartCancel();
            myController->SetError(respMessage->mErrCode, respMessage->mErrInfo);
            if(done) {
                done->Run();
            }
            myController->SetFinished(true);
            return;
        } else {
            if(!(respPb->ParseFromString(respMessage->mPbData))) {
                myController->StartCancel();
                myController->SetError(ERROR_FAILED_SERIALIZE, "serialize error");
                if(done) {
                    done->Run();
                }
                myController->SetFinished(true);
                return;
            }
            INFOLOG("%s | call rpc success, call method name[%s], peer addr[%s], local addr[%s]",
                    respMessage->mMsgId.c_str(),
                    respMessage->mMethodName.c_str(),
                    mClient->GetPeerAddr()->ToString().c_str(),
                    mClient->GetLocalAddr()->ToString().c_str())
            
            if(done) { done->Run(); }
            myController->SetFinished(true);
        }
    }
}

}