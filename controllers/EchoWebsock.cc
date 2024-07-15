#include "EchoWebsock.h"
#include <shared_mutex>
#include <mutex>
#include "Users.h"


using mutex_type = std::shared_mutex;
using read_only_lock = std::shared_lock<mutex_type>;
using updatable_lock = std::unique_lock<mutex_type>;

std::string WebSocketMessageTypeToString(const drogon::WebSocketMessageType &type) {
    switch (type) {
        case drogon::WebSocketMessageType::Text:
            return "Text";
        case drogon::WebSocketMessageType::Binary:
            return "Binary";
        case drogon::WebSocketMessageType::Ping:
            return "Ping";
        case drogon::WebSocketMessageType::Pong:
            return "Pong";
        case drogon::WebSocketMessageType::Close:
            return "Close";
        default:
            return "Unknown";
    }
}


void EchoWebsock::handleNewMessage(const WebSocketConnectionPtr& wsConnPtr, std::string &&message, const WebSocketMessageType &type)
{
     std::string typeStr = WebSocketMessageTypeToString(type);

    // Log the WebSocketMessageType
    LOG_INFO << "Received WebSocket message of type: " << typeStr << "   ----   " << userConnections_.size();

     if (type == WebSocketMessageType::Text)
    {
        // Assume message format is "recipient:message"
        auto colonPos = message.find(':');
        if (colonPos != std::string::npos)
        {
            auto recipient = message.substr(0, colonPos);
            auto msg = message.substr(colonPos + 1);

            auto recipientConn = userConnections_.find(recipient);
            if (recipientConn != userConnections_.end())
            {
                recipientConn->second.socketPtr->send(msg);
            }
        }
    }
}

void EchoWebsock::handleNewConnection(const HttpRequestPtr &req, const WebSocketConnectionPtr& wsConnPtr)
{
    updatable_lock lock(connectionsMutex_);


     // Extract user ID from query parameters (e.g., ws://host/chat?user=userID)
    auto user = req->getParameter("user");

    std::cout << user << "-----";
    if (!user.empty())
    {
        Json::Value userData;

        // Create a callback function to handle the response from getOne
         Json::Value userData;

            // Create a callback function to handle the response from getOne
         auto callback = [wsConnPtr, this, &userData](const HttpResponsePtr &response)
         {
             if (response->statusCode() == k200OK)
             {
                 // Process the response from getOne
                 auto json = response->getJsonObject();
                 if (json)
                 {
                     // Store the user data
                     userData = *json;
                 }
             }
             else
             {
                 // Handle error case
                 wsConnPtr->send("Error retrieving user data", WebSocketMessageType::Text);
             }
         };

         Users fetchedUser(userData);

         userConnections_.emplace(fetchedUser.getValueOfId(), ConnectionInfo(wsConnPtr, fetchedUser));
         //  {
         //     socketPtr= wsConnPtr;
         //  }

         RestfulUsersCtrl restfulUsersCtrl;
         restfulUsersCtrl.getOne(req, std::move(callback), std::move(user));

         Json::Value message;
         Json::Value userList(Json::arrayValue);
         for (const auto &entry : userConnections_)
         {
             userList.append(entry.first);
        }

        message["users"] = userList;

        // Convert the user list to a string
        Json::StreamWriterBuilder writer;
        std::string userListStr = Json::writeString(writer, message);


         for (const auto &entry : userConnections_)
        {
            entry.second.socketPtr->send(userListStr, WebSocketMessageType::Text);
        }
        // Send the list of connected users to the newly connected user
        // wsConnPtr->send(userListStr, WebSocketMessageType::Text);
    }
}

void EchoWebsock::handleConnectionClosed(const WebSocketConnectionPtr& wsConnPtr)
{
    updatable_lock lock(connectionsMutex_);


    LOG_INFO << "Connection Closed";

    for (auto it = userConnections_.begin(); it != userConnections_.end(); ++it)
    {
        LOG_INFO << it->first;
        if (it->second.socketPtr == wsConnPtr)
        {
            userConnections_.erase(it);
            break;
        }
    }
}

void EchoWebsock::sendMessageToUser(const std::vector<std::string> &userIds, const std::string &chatId, const std::string &content) {
    updatable_lock lock(connectionsMutex_);

     int i;

    for (i = 0; i < userIds.size(); i++) {
        auto it = userConnections_.find(userIds[i]);
        for (auto it = userConnections_.begin(); it != userConnections_.end(); ++it)
        {
            
        Json::Value message;
        
        message["content"] = content;
        message["chatId"] = chatId;
        message["userId"] = userIds[i];

        Json::StreamWriterBuilder writer;
        std::string str = Json::writeString(writer, message);

        it->second.socketPtr->send(str, WebSocketMessageType::Text);

        }           
    }
}
