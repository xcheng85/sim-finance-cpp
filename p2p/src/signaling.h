#pragma once

#include <string>
#include <memory>

// http or websocket to implement
class ISignalingClient
{
public:
    virtual ~ISignalingClient() = default;
    virtual void start() = 0;
};

class IMessageSender
{
public:
    virtual void send(const std::string &message) = 0;
};

// first interface deal with WebRTC specs
class IWebRTCSignalingController
{
public:
    virtual ~IWebRTCSignalingController() = default;
    // recv end
    // browser will send "start" through ws
    //  it needs to create session
    virtual void onStart(const std::string &source) = 0;
    virtual void recv(const std::string &message)  = 0;

    // virtual void onClose(const std::string &source) = 0;
    // virtual void onAcceptAnswer(const std::string &source, const std::string &sdp) = 0;
    // virtual void onICECandidates(const std::string &source, const std::vector<ICECandidate> &candidates) = 0;
    // virtual void onAcquire(const std::string &source) = 0;
    // send endWebRTCSignalingController:string &destination, const std::vector<ICECandidate> &candidates) = 0;
};

class IWebsocketEventHandler
{
public:
    // event handler needs a channel to send out after processing
    virtual void onHandshake(IMessageSender *) = 0;
    virtual void onRecvMessage(IMessageSender *, const std::string &message) = 0;
    virtual void onDisconnect(IMessageSender *) = 0;
};

// business logic
// hub of receiving and sending messages.
class IMessageBroker
{
public:
    virtual ~IMessageBroker() = default;
    virtual void recv(const std::string &message) = 0;
    virtual void send(const std::string &message) = 0;
};

// ioc of websocket impl
class WebsocketSignalingClient : public ISignalingClient, public IWebsocketEventHandler
{
public:
    WebsocketSignalingClient() = delete;
    explicit WebsocketSignalingClient(const std::string &hostname, const std::string &port, const std::string &path)
        : _hostname(hostname), _port(port), _path(path)
    {
    }

    WebsocketSignalingClient(const WebsocketSignalingClient &) = delete;
    virtual ~WebsocketSignalingClient() override = default;

    virtual void start() override;
    virtual void onHandshake(IMessageSender *) override;
    virtual void onRecvMessage(IMessageSender *, const std::string &message) override;
    virtual void onDisconnect(IMessageSender *) override;

private:
    std::string _hostname;
    std::string _port;
    std::string _path;
    // std::unique_ptr<IMessageBroker> _msgBroker{nullptr}; // can be reused by all the IWebRTCSignalingController
    std::unique_ptr<IWebRTCSignalingController> _webrtcController{nullptr};
};

// class WebSocketClient
// {
// public:
//     // ws://127.0.0.1/fromRenderer
//     WebSocketClient() = delete;
//     explicit WebSocketClient(const std::string &hostname, const std::string &port, const std::string &path);
//     explicit WebSocketClient(const WebSocketClient &) = delete;
//     ~WebSocketClient();

//     void connect();

//     //     // std::unique_ptr<SignalingInstanceInterface> CreateInstance(
//     //     //     std::unique_ptr<SignalingControlInterface> control) override;

//     // private:
//     //     std::unique_ptr<SignalingStrategy> strategy_;
// };
