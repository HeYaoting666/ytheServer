#include "rpc_controller.h"

namespace ythe {

void RpcController::Reset() {
    mErrorCode = 0;
    mErrorInfo = "";
    mMsgId = "";

    mIsFailed = false;
    mIsCanceled = false;
    mIsFinished = false;

    mLocalAddr = nullptr;
    mPeerAddr = nullptr;

    mTimeout = 1000;   // ms
}

bool RpcController::Failed() const {
    return mIsFailed;
}

std::string RpcController::ErrorText() const {
    return mErrorInfo;
}

void RpcController::StartCancel() {
    mIsCanceled = true;
    mIsFailed   = true;
    mIsFinished = true;
}

void RpcController::SetFailed(const std::string& reason) {
    mErrorInfo = reason;
    mIsFailed = true;
}

bool RpcController::IsCanceled() const {
    return mIsCanceled;
}

void RpcController::NotifyOnCancel(google::protobuf::Closure* callback) {
}

bool RpcController::Finished() const {
    return mIsFinished;
}

void RpcController::SetError(int32_t errorCode, const std::string& errorInfo) {
    mErrorCode = errorCode;
    mErrorInfo = errorInfo;
    mIsFailed  = true;
}

void RpcController::SetMsgId(const std::string& msgId) {
    mMsgId = msgId;
}

void RpcController::SetLocalAddr(const IPNetAddr::sp& addr) {
    mLocalAddr = addr;
}

void RpcController::SetPeerAddr(const IPNetAddr::sp& addr) {
    mPeerAddr = addr;
}

void RpcController::SetTimeout(int timeout) {
    mTimeout = timeout;
}

void RpcController::SetFinished(bool value) {
    mIsFinished = value;
}

std::string RpcController::GetMsgId() {
    return mMsgId;
}

int32_t RpcController::GetErrorCode() const {
    return mErrorCode;
}

std::string RpcController::GetErrorInfo() const {
    return mErrorInfo;
}

IPNetAddr::sp RpcController::GetLocalAddr() {
    return mLocalAddr;
}

IPNetAddr::sp RpcController::GetPeerAddr() {
    return mPeerAddr;
}

int RpcController::GetTimeout() const {
    return mTimeout;
}

}