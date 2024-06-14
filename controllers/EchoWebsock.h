#pragma once

#include <drogon/WebSocketController.h>

using namespace drogon;

class EchoWebsock : public drogon::WebSocketController<EchoWebsock>
{
  public:
     void handleNewMessage(const WebSocketConnectionPtr&,
                                  std::string &&,
                                  const WebSocketMessageType &) override;
    void handleNewConnection(const HttpRequestPtr &,
                                     const WebSocketConnectionPtr&) override;
    void handleConnectionClosed(const WebSocketConnectionPtr&) override;
    WS_PATH_LIST_BEGIN
    // list path definitions here;
    // WS_PATH_ADD("/path", "filter1", "filter2", ...);
    WS_PATH_ADD("/chat");
    WS_PATH_LIST_END

    private:
      std::unordered_map<std::string, WebSocketConnectionPtr> userConnections_;
};

