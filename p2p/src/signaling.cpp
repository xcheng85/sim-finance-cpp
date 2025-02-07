#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <utility>

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/strand.hpp>
#include <boost/signals2.hpp>

#include "signaling.h"

// #include <webrtc.h>
// #include <p2pClient.h>

namespace beast = boost::beast;
namespace http = beast::http;
namespace websocket = beast::websocket;
using tcp = boost::asio::ip::tcp;

class WebRTCSignalingController : public IWebRTCSignalingController
{
public:
    typedef boost::signals2::signal<
        bool(IWebRTCSignalingController *controller,
             const std::string &source,
             const std::string &parameter)>
        signal_type;

    WebRTCSignalingController();
    ~WebRTCSignalingController();

    virtual void onStart(const std::string &source) override;
    virtual void recv(const std::string &message) override;

private:
    // P2pClient *_p2pClient{nullptr};
    // std::unordered_map<std::string, rtc::scoped_refptr<Session>> _p2pSessionMap;
    std::unordered_map<std::string, signal_type> _signalMap;
    std::vector<boost::signals2::connection> _connectionMap;
};

WebRTCSignalingController::WebRTCSignalingController()
{
    // _p2pClient = new P2pClient();

    _signalMap.insert(std::make_pair("start", signal_type()));
    _connectionMap.emplace_back(_signalMap["start"].connect(
        [](
            IWebRTCSignalingController *controller,
            const std::string &source,
            const std::string &parameter)
        {
            controller->onStart(source);
            return true;
        }));
}

WebRTCSignalingController::~WebRTCSignalingController()
{
    for (const auto &c : _connectionMap)
    {
        c.disconnect();
    }

    // delete _p2pClient;
    // _p2pClient = nullptr;
}

void WebRTCSignalingController::onStart(const std::string &source)
{
    // source uuid
    // _p2pSessionMap[source] = _p2pClient->createSession();
}

void WebRTCSignalingController::recv(const std::string &message)
{
    // TRIGGER THE boost signal slot
}

// an implication through ws
class MessageBroker : public IMessageBroker
{
public:
    explicit MessageBroker(IMessageSender *msgSender);
    MessageBroker(const MessageBroker &) = delete;
    virtual ~MessageBroker();

    virtual void recv(const std::string &message) override;
    virtual void send(const std::string &message) override;

private:
    // not an owner of sender
    IMessageSender *_msgSender;
};

MessageBroker::MessageBroker(IMessageSender *msgSender)
    : _msgSender(msgSender)
{
}

MessageBroker::~MessageBroker()
{
}

void MessageBroker::recv(const std::string &message)
{
}

void MessageBroker::send(const std::string &message)
{
    _msgSender->send(message);
}

// reference: https://www.boost.org/doc/libs/develop/libs/beast/example/websocket/client/async/websocket_client_async.cpp
// Report a failure
void fail(beast::error_code ec, char const *what)
{
    std::cerr << what << ": " << ec.message() << "\n";
}

// Sends a WebSocket message and prints the response
class session : public std::enable_shared_from_this<session>, public IMessageSender
{
private:
    tcp::resolver _resolver;
    websocket::stream<beast::tcp_stream> _ws;
    beast::flat_buffer _buffer;
    std::string _host;
    std::string _path;

    int _pingpongState{0}; // 0: ping, 1: pong
    boost::asio::steady_timer _timer;

    // bridge between ws event and signaling domain-specific event, an intepretation layer
    IWebsocketEventHandler *_eventController;

    // for automatica retry logic
    std::vector<boost::shared_ptr<std::string const>> _pendingSendQueue;

public:
    // Resolver and socket require an io_context
    // By binding the handlers to the same io_context::strand, we are ensuring that they cannot execute concurrently.
    // async but sequential for resolver, ws and timer, three async + sequential flow, still multi-threaded
    explicit session(boost::asio::io_context &ctx)
        : _resolver(boost::asio::make_strand(ctx)),
          _ws(boost::asio::make_strand(ctx)),
          _timer(boost::asio::make_strand(ctx), (std::chrono::steady_clock::time_point::max)()) // not expire, set it max
    {
    }

    ~session()
    {
        _eventController->onDisconnect(this);
    }

    // Start the asynchronous operation
    void bootstrap(const std::string &host,
                   const std::string &port,
                   const std::string &path,
                   IWebsocketEventHandler *eventController)
    {
        // Save these for later
        _host = host;
        _path = path;
        _eventController = eventController;
        // Look up the domain name
        _resolver.async_resolve(host, port,
                                beast::bind_front_handler(&session::onResolve, shared_from_this()));
    }

    void onResolve(beast::error_code ec, tcp::resolver::results_type results)
    {
        if (ec)
            return fail(ec, "resolve");

        // Set the timeout for the operation: ws timeout
        beast::get_lowest_layer(_ws).expires_after(std::chrono::seconds(30));

        // Make the connection on the IP address we get from a lookup
        // resolve result is the ip address
        beast::get_lowest_layer(_ws).async_connect(
            results,
            beast::bind_front_handler(
                &session::onConnect,
                shared_from_this()));
    }

    void onConnect(beast::error_code ec, tcp::resolver::results_type::endpoint_type ep)
    {
        if (ec)
            return fail(ec, "connect");

        // Turn off the timeout on the tcp_stream, because
        // the websocket stream has its own timeout system.
        beast::get_lowest_layer(_ws).expires_never();

        // Set suggested timeout settings for the websocket
        _ws.set_option(
            websocket::stream_base::timeout::suggested(
                beast::role_type::client));

        // Set a decorator to change the User-Agent of the handshake
        // change a header
        _ws.set_option(websocket::stream_base::decorator(
            [](websocket::request_type &req)
            {
                req.set(http::field::user_agent,
                        std::string(BOOST_BEAST_VERSION_STRING) +
                            " websocket-client-async");
            }));

        // //    // Update the host_ string. This will provide the value of the
        // //         // Host HTTP header during the WebSocket handshake.
        // //         // See https://tools.ietf.org/html/rfc7230#section-5.4
        // _host += ':' + std::to_stqueue_handshake
        _ws.async_handshake(_host, _path,
                            beast::bind_front_handler(
                                &session::onHandshake,
                                shared_from_this()));
    }

    // this needs to be programmed based on the use cases
    void onHandshake(beast::error_code ec)
    {
        if (ec)
            return fail(ec, "handshake");

        // Read a message into our buffer
        _ws.async_read(
            _buffer,
            beast::bind_front_handler(
                &session::onRead,
                shared_from_this()));

        // reference: https://livequeue_.boost.org/doc/libs/1_81_0/libs/beast/doc/html/beast/using_websocket/control_frames.html
        // register callback for control frames: ping, pong and close
        _ws.control_callback(
            std::bind(
                &session::onControlFrame,
                shared_from_this(),
                std::placeholders::_1, // kind: ping,pong or close
                std::placeholders::_2));

        // ws heartbeat
        // use timer async
        // https://think-async.com/Asio/asio-1.30.2/doc/asio/tutorial/tuttimer4.html
        _timer.expires_after(std::chrono::seconds(1));
        // start timer for ping
        _timer.async_wait(
            boost::asio::bind_executor(
                _timer.get_executor(),
                std::bind(
                    &session::onTimer,
                    shared_from_this(),
                    std::placeholders::_1)));

        _eventController->onHandshake(this);
    }

    void onRead(beast::error_code ec, std::size_t bytes_transferred)
    {
        if (ec)
        {
            _timer.cancel();
            return fail(ec, "websocket read");

            // never close

            // // Close the WebSocket connection
            // ws_.async_close(websocket::close_code::normal,
            //     beast::bind_front_handler(
            //         &session::on_close,
            //         shared_from_this()));
        }
        // ws read into buffer: flatbuffer: A linear dynamic buffer.
        auto payload = beast::buffers_to_string(_buffer.data());
        _buffer.consume(_buffer.size());

        // some post-processing to the string: todo
        _eventController->onRecvMessage(this, payload);

        // pattern: create another async task
        _ws.async_read(
            _buffer,
            beast::bind_front_handler(
                &session::onRead,
                shared_from_this()));
    }

    void onControlFrame(websocket::frame_type kind, boost::beast::string_view payload)
    {
        boost::ignore_unused(kind, payload);

        if (kind == websocket::frame_type::pong)
        {
            std::cout << "PONG" << std::endl;
            // restart heartbeat
            this->_pingpongState = 0;
        }
    }

    // ws heartbeat
    void onTimer(boost::system::error_code ec)
    {
        if (ec == boost::asio::error::operation_aborted)
            return;
        if (ec)
        {
            return fail(ec, "timer");
        }
        // See if the timer really expired since the deadline may have moved.
        if (_timer.expiry() <= std::chrono::steady_clock::now())
        {
            if (_ws.is_open() && _pingpongState == 0)
            {
                _pingpongState = 1;
                _ws.async_ping({}, boost::asio::bind_executor(
                                       _ws.get_executor(),
                                       std::bind(
                                           &session::onPing,
                                           shared_from_this(),
                                           std::placeholders::_1)));
            }
        }

        // every 2 seconds for heartbeat
        _timer.expires_after(std::chrono::seconds(5));
        _timer.async_wait(boost::asio::bind_executor(_timer.get_executor(),
                                                     std::bind(
                                                         &session::onTimer,
                                                         shared_from_this(),
                                                         std::placeholders::_1)));
    }

    void onPing(beast::error_code ec)
    {
        if (ec == boost::asio::error::operation_aborted)
            return;

        if (ec)
            return fail(ec, "ping");
        std::cout << "PING" << std::endl;
        if (_pingpongState == 1)
        {
            _pingpongState = 2;
        }
    }

    void send(boost::shared_ptr<std::string const> const &ss)
    {
        // Submits a completion token or function object for execution.
        boost::asio::post(
            _ws.get_executor(),
            beast::bind_front_handler(
                &session::onSend,
                shared_from_this(),
                ss));
    }

    void onSend(boost::shared_ptr<std::string const> const &ss)
    {
        _pendingSendQueue.push_back(ss);
        if (_pendingSendQueue.size() > 1)
            return;
        _ws.async_write(
            boost::asio::buffer(*_pendingSendQueue.front()),
            beast::bind_front_handler(
                &session::onWrite,
                shared_from_this()));
    }

    void onWrite(beast::error_code ec, std::size_t bytes_transferred)
    {
        if (ec)
            return fail(ec, "websocket write");

        // remove from the pending queue
        _pendingSendQueue.erase(_pendingSendQueue.begin());
        // retry logic until the queue is clean
        if (!_pendingSendQueue.empty())
            _ws.async_write(
                boost::asio::buffer(*_pendingSendQueue.front()),
                beast::bind_front_handler(
                    &session::onWrite,
                    shared_from_this()));
    }

    // way to send some message to the signaling server, here is the client
    virtual void send(const std::string &str)
    {
        send(boost::make_shared<std::string const>(str));
    }
};

// blocking call
void runWebsocketClientAsync(const std::string &host, const std::string &port, const std::string &path, IWebsocketEventHandler *eventController)
{
    boost::asio::io_context ctx;

    // Launch the asynchronous operation
    std::make_shared<session>(ctx)->bootstrap(host, port, path, eventController);

    // only return when there is 0 async task (when heartbeat task is 0, when error occurs, ws closes)
    ctx.run();
}

void WebsocketSignalingClient::start()
{
    runWebsocketClientAsync(_hostname, _port, _path, this);
}

void WebsocketSignalingClient::onHandshake(IMessageSender *sender)
{
    // stateless
    auto msgBroker = std::make_unique<MessageBroker>(sender);

    // create a new session
    _webrtcController = std::make_unique<WebRTCSignalingController>();
}

void WebsocketSignalingClient::onRecvMessage(IMessageSender *sender, const std::string &message)
{
    // assert(_msgBroker);
    _webrtcController->recv(message);
}

void WebsocketSignalingClient::onDisconnect(IMessageSender *sender)
{
    // trigger dctor of unique_ptr
    // assert(_msgBroker);
    // _msgBroker = nullptr;
    _webrtcController = nullptr;
}