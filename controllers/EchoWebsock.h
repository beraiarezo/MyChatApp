#pragma once

#include <drogon/WebSocketController.h>

using namespace drogon;

class EchoWebsock : public drogon::WebSocketController<EchoWebsock>
{
  public:
    // static EchoWebsock& getInstance() {
    //     static EchoWebsock instance;
    //     return instance;
    // };

    EchoWebsock() {
      std::cout << "EchoWebsock constructor called" << std::endl;
    }

    void handleNewMessage(const WebSocketConnectionPtr&,
                                  std::string &&,
                                  const WebSocketMessageType &) override;
    void handleNewConnection(const HttpRequestPtr &,
                                     const WebSocketConnectionPtr&) override;
    void handleConnectionClosed(const WebSocketConnectionPtr&) override;

    void sendMessageToUser(const std::string &userId, const std::string &message);

    WS_PATH_LIST_BEGIN
      WS_PATH_ADD("/chat", Get);
    WS_PATH_LIST_END

    private:
        // EchoWebsock() = default;
        std::unordered_map<std::string, WebSocketConnectionPtr> userConnections_;
        std::mutex connectionsMutex_;
};

