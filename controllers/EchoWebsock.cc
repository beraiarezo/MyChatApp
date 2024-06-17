#include "EchoWebsock.h"


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
    LOG_INFO << "Received WebSocket message of type: " << typeStr;

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
                recipientConn->second->send(msg);
            }
        }
    }
}

void EchoWebsock::handleNewConnection(const HttpRequestPtr &req, const WebSocketConnectionPtr& wsConnPtr)
{
     // Extract user ID from query parameters (e.g., ws://host/chat?user=userID)
    auto user = req->getParameter("user");

    LOG_DEBUG << "------" << user << "----USSEEERRRR";

    if (!user.empty())
    {
        userConnections_[user] = wsConnPtr;
    }
}

void EchoWebsock::handleConnectionClosed(const WebSocketConnectionPtr& wsConnPtr)
{
    LOG_INFO << "Connection Closed";

    for (auto it = userConnections_.begin(); it != userConnections_.end(); ++it)
    {
        LOG_INFO << it->first;
        if (it->second == wsConnPtr)
        {
            userConnections_.erase(it);
            break;
        }
    }
}

void EchoWebsock::sendMessageToUser(const std::string &userId, const std::string &message) {
    std::lock_guard<std::mutex> guard(connectionsMutex_);
    auto it = userConnections_.find(userId);
    if (it != userConnections_.end()) {
        it->second->send(message);
    }
}
