#include "connection.h"
#include "src/log/logger.h"
#include "src/coder/tinypb_coder.h"
#include "src/coder/tinypb_protocol.h"
#include "src/rpc/rpc_dispatcher.h"
#include "src/net/fd_event.h"

namespace ythe {

TCPConnection::TCPConnection(FdEvent* fdEvent, EventLoop* eventLoop, int bufferSize, 
    IPNetAddr::sp localAddr, IPNetAddr::sp peerAddr, TCPConnectionType type)
    : mConnEvent(fdEvent), mEventLoop(eventLoop), mLocalAddr(localAddr), mPeerAddr(peerAddr), mType(type)
{
    // 初始化连接状态
    mState = NotConnected;

    // 初始化接收发送缓冲区
    mRecvBuffer = std::make_shared<TCPBuffer>(bufferSize);
    mSendBuffer = std::make_shared<TCPBuffer>(bufferSize);

    // 监听可读事件
    ListenReadEvent();

    mCoder = new TinyPBCoder();
}

TCPConnection::~TCPConnection()
{
    if(mCoder != nullptr) {
        delete mCoder;
        mCoder = nullptr;
    }
}

void TCPConnection::ListenReadEvent(bool setET)
{
    mConnEvent->SetFdEvent(IN_EVENT, std::bind(&TCPConnection::onRead, this));
    if(setET)
        mConnEvent->SetEpollET();
    mEventLoop->AddFdEventToEpoll(mConnEvent);
}

void TCPConnection::ListenWriteEvent(bool setET)
{
    mConnEvent->SetFdEvent(OUT_EVENT, std::bind(&TCPConnection::onWrite, this));
    if(setET)
        mConnEvent->SetEpollET();
    mEventLoop->AddFdEventToEpoll(mConnEvent);
}

void TCPConnection::CancelListenReadEvent(bool cancelET)
{
    mConnEvent->CancelFdEvent(IN_EVENT);
    if(cancelET)
        mConnEvent->CancelEpollET();
    mEventLoop->AddFdEventToEpoll(mConnEvent);
}

void TCPConnection::CancelListenWriteEvent(bool cancelET)
{
    mConnEvent->CancelFdEvent(OUT_EVENT);
    if(cancelET)
        mConnEvent->CancelEpollET();
    mEventLoop->AddFdEventToEpoll(mConnEvent);
}

void TCPConnection::onRead()
{
    if (mState != Connected) {
        ERRORLOG("fd[%d], onRead error, client has already disconnected, addr[%s]", mConnEvent->GetFd(), mPeerAddr->ToString().c_str())
        return;
    }

    bool isClose = false;
    while(true) {
        if(mRecvBuffer->WriteAble() == 0) {
            mRecvBuffer->ResizeBuffer(2 * mRecvBuffer->Size());
        }

        int writeSize = mRecvBuffer->WriteAble();
        int writeStart = mRecvBuffer->WriteIndex();
        // fd socket 内核缓冲区 --> recvBuffer 应用层缓冲区
        int ret = read(mConnEvent->GetFd(), mRecvBuffer->Data() + writeStart, writeSize);
        DEBUGLOG("fd[%d] success read %d bytes from addr[%s]", mConnEvent->GetFd(), ret, mPeerAddr->ToString().c_str())
        if (ret == 0) {  // 对端断开连接
            isClose = true;
            break;
        }
        if (ret == -1) {
            if(errno == EAGAIN || errno == EWOULDBLOCK) // 全部读取完成
                break;
            else {
                ERRORLOG("fd[%d] onRead Error from addr[%s]", mConnEvent->GetFd(), mPeerAddr->ToString().c_str())
                return;
            }
        }
        mRecvBuffer->MoveWriteIndex(ret);
    }
    if(isClose) {
        INFOLOG("fd[%d] peer closed, peer addr [%s]]", mConnEvent->GetFd(), mPeerAddr->ToString().c_str())
        clear();
        return;
    }

    if(mType == TCPConnectionByClient) {
        mEventLoop->Stop();
        return;
    }

    if(mType == TCPConnectionByServer) {
        // 执行http任务或rpc任务
        execute();
    }
}

void TCPConnection::onWrite()
{
    if (mState != Connected) {
        ERRORLOG("fd[%d] onWrite error, disconnected, addr[%s]", mConnEvent->GetFd(), mPeerAddr->ToString().c_str())
        return;
    }

    bool isWriteAll = false;
    while(true) {
        if (mSendBuffer->ReadAble() == 0) {
            DEBUGLOG("%s", "no data need to send")
            isWriteAll = true;
            break;
        }

        int readSize = mSendBuffer->ReadAble();
        int readStart = mSendBuffer->ReadIndex();
        // sendBuffer 应用层缓冲区 --> fd socket 内核缓冲区
        int ret = write(mConnEvent->GetFd(), mSendBuffer->Data() + readStart, readSize) ;
        if(ret == -1 && errno == EAGAIN) {
            // 发送缓冲区已满，不能再发送了。
            // 等待下次 fd 可写的时候再次发送数据即可
            ERRORLOG("%s", "write data error, errno==EAGAIN and rt == -1")
            break;
        }
        DEBUGLOG("fd[%d] success write %d bytes to addr[%s]", mConnEvent->GetFd(), ret, mPeerAddr->ToString().c_str())
        mSendBuffer->MoveReadIndex(ret);
    }
    if(isWriteAll) {
        CancelListenWriteEvent();
    }
}

void TCPConnection::execute()
{
    std::vector<AbstractProtocol::sp> reqMessages;
    std::vector<AbstractProtocol::sp> respMessages;

    // 从 RecvBuffer 里 decode 得到 reqMessages 对象
    mCoder->Decode(mRecvBuffer, reqMessages);
    for(auto& req: reqMessages) {
        INFOLOG("success get request[%s] from client[%s]", req->mMsgId.c_str(), mPeerAddr->ToString().c_str())

        // 针对每一个请求，调用 rpc 方法，获取 resp
        std::shared_ptr<TinyPBProtocol> resp = std::make_shared<TinyPBProtocol>();
        RpcDispatcher::GetInstance()->Dispatch(req, resp, mLocalAddr, mPeerAddr);
        respMessages.emplace_back(resp);
    }
    // 消息写回，将响应体 respMessages 放入到发送缓冲区，监听可写事件回包
    mCoder->Encode(respMessages, mSendBuffer);
    ListenWriteEvent();
}

void TCPConnection::clear()
{
    if(mState == Closed)
        return;

    mEventLoop->DeleteFdEventFromEpoll(mConnEvent);
    mState = Closed;
}

}