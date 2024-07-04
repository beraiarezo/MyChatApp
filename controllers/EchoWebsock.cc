#include "EchoWebsock.h"
#include <shared_mutex>
#include <mutex>




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
                // recipientConn->second->send(msg);
            }
        }
    }
}

void EchoWebsock::handleNewConnection(const HttpRequestPtr &req, const WebSocketConnectionPtr& wsConnPtr)
{
    updatable_lock lock(connectionsMutex_);


     // Extract user ID from query parameters (e.g., ws://host/chat?user=userID)
    auto userId = req->getParameter("user");
    auto dbClientPtr = getDbClient();
    // Users
    drogon::orm::Mapper<Users> mapper(dbClientPtr);

    if (!userId.empty())
    {
        UserData userData;
        userData.id = userId;
        userData.connection = wsConnPtr;
        mapper.findByPrimaryKey(userId, [&userData](const drogon_model::chat::Users &user) {
            userData.name = user.getValueOfName();
            userData.email = user.getValueOfEmail(); }, [](const DrogonDbException &e)
                                { std::cerr << "Error: " << e.base().what() << std::endl; });

        // userConnections_[user] = wsConnPtr;
        //   userConnections_[user] = {
        //         connection: wsConnPtr,
        //         name : "rezi",
        //         email: "r@g.com",
        //         id: "rezsi"
        //     };

    }
}

[5,2,,3,4,7]

// void EchoWebsock::handleConnectionClosed(const WebSocketConnectionPtr& wsConnPtr)
// {
//     updatable_lock lock(connectionsMutex_);


//     LOG_INFO << "Connection Closed";

//     for (auto it = userConnections_.begin(); it != userConnections_.end(); ++it)
//     {
//         LOG_INFO << it->first;
//         if (it->second == wsConnPtr)
//         {
//             userConnections_.erase(it);
//             break;
//         }
//     }
// }

// void EchoWebsock::sendMessageToUser(const std::string &userId, const std::string &message) {
//             updatable_lock lock(connectionsMutex_);

//     auto it = userConnections_.find(userId);

//     for (auto it = userConnections_.begin(); it != userConnections_.end(); ++it)
//     {


//         it->second->send("8681f5e9-e2fd-434e-9ca2-b774cd39fbcd:hello");
//     }
// }
