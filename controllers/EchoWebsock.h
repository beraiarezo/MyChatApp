#pragma once

#include <drogon/WebSocketController.h>

using namespace drogon;

class EchoWebsock : public drogon::WebSocketController<EchoWebsock>
{
  public:
    static EchoWebsock& getInstance() {
        static EchoWebsock instance;
        return instance;
    };

    void handleNewMessage(const WebSocketConnectionPtr&,
                                  std::string &&,
                                  const WebSocketMessageType &) override;
    void handleNewConnection(const HttpRequestPtr &,
                                     const WebSocketConnectionPtr&) override;
    void handleConnectionClosed(const WebSocketConnectionPtr&) override;

    void sendMessageToUser(const std::string &userId, const std::string &message);

    WS_PATH_LIST_BEGIN
    // list path definitions here;
    // WS_PATH_ADD("/path", "filter1", "filter2", ...);
    WS_PATH_ADD("/chat");
    WS_PATH_LIST_END

    private:
        EchoWebsock() = default;
        std::unordered_map<std::string, WebSocketConnectionPtr> userConnections_;
        std::mutex connectionsMutex_;

      // std::unordered_map<std::string, WebSocketConnectionPtr> userConnections_;
};

