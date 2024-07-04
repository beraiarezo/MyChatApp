#pragma once

#include <drogon/WebSocketController.h>
#include <shared_mutex>
#include <mutex>
#include "models/Users.h"

using mutex_type = std::shared_mutex;
using read_only_lock = std::shared_lock<mutex_type>;
using updatable_lock = std::unique_lock<mutex_type>;

using namespace drogon;
using namespace drogon::orm;
using namespace drogon_model::chat;


struct UserData {
    drogon::WebSocketConnectionPtr connection;
    std::string id;
    std::string name;
    std::string email;
    // Add other user-related fields as needed
};

class EchoWebsock : public drogon::WebSocketController<EchoWebsock, false>
{
  public:     
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


    orm::DbClientPtr getDbClient() {
      return drogon::app().getDbClient("default");
    }

    std::unordered_map<std::string, UserData> getUserConnections() {
      read_only_lock lock(connectionsMutex_);
      return userConnections_;
    }

    std::shared_mutex* getConnectionMutex() {
        return &connectionsMutex_;
    }

    private:
      std::unordered_map<std::string, UserData> userConnections_;
      mutex_type connectionsMutex_;
};

