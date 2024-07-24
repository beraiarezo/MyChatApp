#pragma once

#include <drogon/WebSocketController.h>
#include <shared_mutex>
#include <mutex>
#include "RestfulUsersCtrl.h"
#include "Users.h"

using mutex_type = std::shared_mutex;
using read_only_lock = std::shared_lock<mutex_type>;
using updatable_lock = std::unique_lock<mutex_type>;

using namespace drogon;

struct ConnectionInfo {
  drogon::WebSocketConnectionPtr socketPtr;
  Users user;

  ConnectionInfo(const drogon::WebSocketConnectionPtr &ptr, const Users &usr)
      : socketPtr(ptr), user(usr) {}
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

    void sendMessageToUser(const std::vector<std::string>&userIds, const std::string &chatId, const std::string &content);

    WS_PATH_LIST_BEGIN
      WS_PATH_ADD("/chat", Get);
    WS_PATH_LIST_END


    std::unordered_map<std::string, ConnectionInfo> getUserConnections() {
      read_only_lock lock(connectionsMutex_);
      return userConnections_;
    }

    std::shared_mutex* getConnectionMutex() {
        return &connectionsMutex_;
    }

    private:
      std::unordered_map<std::string, ConnectionInfo> userConnections_;
      mutex_type connectionsMutex_;
};

