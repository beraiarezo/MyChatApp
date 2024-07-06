#include "ConversationCtrl.h"
#include "EchoWebsock.h"
#include <coroutine>
#include <shared_mutex>
#include <mutex>

using mutex_type = std::shared_mutex;
using read_only_lock = std::shared_lock<mutex_type>;
using updatable_lock = std::unique_lock<mutex_type>;

using namespace drogon;


// Function to check if chat exists and retrieve its ID
Task<std::optional<std::string>> getChatIdIfExists(const std::vector<std::string> &userIds) {
    auto dbClient = app().getDbClient();
    try {
        auto result = co_await dbClient->execSqlCoro(
            "SELECT chat_id FROM users_chats WHERE user_id IN ($1, $2) GROUP BY chat_id HAVING COUNT(DISTINCT user_id) = 2",
            userIds[0], userIds[1]
        );

        if (!result.empty()) {
            co_return result[0]["chat_id"].as<std::string>();
        } else {
            co_return std::nullopt;
        }
    } catch (const orm::DrogonDbException &e) {
        LOG_ERROR << "Error checking chat existence: " << e.base().what();
        throw;
    }
}

Task<drogon::orm::Row> createChat(const std::string &creator_id) {
    auto dbClient = app().getDbClient();
    try {
        auto result = co_await dbClient->execSqlCoro("INSERT INTO chats (creator_id, created_at) VALUES ($1, CURRENT_TIMESTAMP) RETURNING id", creator_id);
        co_return result[0];
    } catch (const orm::DrogonDbException &e) {
        LOG_ERROR << "Error creating conversation: " << e.base().what();
        throw;
    }
}

Task<void> addParticipants(const std::string &chat_id, const std::vector<std::string> &userIds) {
    auto dbClient = app().getDbClient();
    try {
        for (const auto &userId : userIds) {
            co_await dbClient->execSqlCoro("INSERT INTO users_chats (chat_id, user_id, joined_at) VALUES ($1, $2, CURRENT_TIMESTAMP)", chat_id, userId);
        }
    } catch (const orm::DrogonDbException &e) {
        LOG_ERROR << "Error adding participants: " << e.base().what();
        throw;
    }
}

Task<drogon::orm::Row> addMessage(const std::string &chat_id, const std::string &sender_id, const std::string &content) {
    auto dbClient = app().getDbClient();
    try {
        auto result = co_await dbClient->execSqlCoro("INSERT INTO messages (chat_id, sender_id, content, created_at) VALUES ($1, $2, $3, CURRENT_TIMESTAMP) RETURNING id", chat_id, sender_id, content);
        co_return result[0];
    } catch (const orm::DrogonDbException &e) {
        LOG_ERROR << "Error adding message: " << e.base().what();
        throw;
    }
}

Task<void> ConversationCtrl::handleNewChat(
    const std::vector<std::string> &userIds,
    const std::string &messageContent,
    const std::string &senderId) {
    auto dbClient = app().getDbClient();
    bool exceptionOccurred = false;

    try {
        // Perform the transaction
        co_await dbClient->execSqlCoro("BEGIN");

        // Create Chat
        auto chatRow = co_await createChat(senderId);
        std::string chatId = chatRow["id"].as<std::string>();

        // Add Participants
        co_await addParticipants(chatId, userIds);

        // Add a Message to Chat
        auto messageRow = co_await addMessage(chatId, senderId, messageContent);
        std::string messageId = messageRow["id"].as<std::string>();

        // Notify users about the new message
        co_await notifyUsers(chatId, userIds, messageContent);

        // Commit transaction
        co_await dbClient->execSqlCoro("COMMIT");
    } catch (const orm::DrogonDbException &e) {
        LOG_ERROR << "Error handling new chat: " << e.base().what();
        exceptionOccurred = true;
    }

    if (exceptionOccurred) {
        try {
            co_await dbClient->execSqlCoro("ROLLBACK");
        } catch (const orm::DrogonDbException &rollbackError) {
            LOG_ERROR << "Error during rollback: " << rollbackError.base().what();
            // Optionally handle rollback error
        }
        throw;
    }
}

Task<void> ConversationCtrl::notifyUsers(const std::string &chat_id, const std::vector<std::string> &userIds, const std::string &message) {
    for (const auto &userId : userIds) {
        echoWebsock_->sendMessageToUser(userIds, "You have received a new message in chat " + chat_id + ": " + message);
    }
    co_return;
}

Task<void> ConversationCtrl::handleNewMessage(
    const std::string &chat_id,
    const std::string &messageContent,
    const std::string &senderId,
    const std::vector<std::string> &userIds) {
    auto dbClient = app().getDbClient();
    bool exceptionOccurred = false;

    try {
        // Add a Message to Chat
        auto messageRow = co_await addMessage(chat_id, senderId, messageContent);
        std::string messageId = messageRow["id"].as<std::string>();

                echoWebsock_->sendMessageToUser(userIds, "You have received a new message in chat " + chat_id + ": " + messageContent);


        // Notify users about the new message
        co_await notifyUsers(chat_id, userIds, messageContent);
    } catch (const orm::DrogonDbException &e) {
        LOG_ERROR << "Error handling new message: " << e.base().what();
        exceptionOccurred = true;
    }

    if (exceptionOccurred) {
        try {
            co_await dbClient->execSqlCoro("ROLLBACK");
        } catch (const orm::DrogonDbException &rollbackError) {
            LOG_ERROR << "Error during rollback: " << rollbackError.base().what();
            // Optionally handle rollback error
        }
        throw;
    }
}

Task<void> ConversationCtrl::handleRequestAsync(const HttpRequestPtr &req, std::function<void(const HttpResponsePtr &)> callback) {
    // Parse request JSON
    auto json = req->getJsonObject();
    if (!json) {
        auto resp = HttpResponse::newHttpResponse();
        resp->setStatusCode(HttpStatusCode::k400BadRequest);
        resp->setBody("Invalid JSON format");
        callback(resp);
        co_return;
    }

    // Extract data from JSON
    auto userIdsJson = (*json)["userIds"];
    std::vector<std::string> userIds;
    for (const auto &userId : userIdsJson) {
        userIds.push_back(userId.asString());
    }
    std::string messageContent = (*json)["messageContent"].asString();
    std::string senderId = (*json)["senderId"].asString();

    try {
        // Check if chat already exists between the users
        auto existingChatId = co_await getChatIdIfExists(userIds);

        if (existingChatId.has_value()) {
            // Handle new message in existing chat
            co_await handleNewMessage(existingChatId.value(), messageContent, senderId, userIds);
        } else {
            // Handle new chat creation and message
            co_await handleNewChat(userIds, messageContent, senderId);
        }

        auto resp = HttpResponse::newHttpResponse();
        resp->setBody("Message processed successfully!");
        callback(resp);
    } catch (const std::exception &e) {
        auto resp = HttpResponse::newHttpResponse();
        resp->setStatusCode(HttpStatusCode::k500InternalServerError);
        resp->setBody("Error processing message");
        callback(resp);
    }
}

void ConversationCtrl::handler(const HttpRequestPtr &req, std::function<void(const HttpResponsePtr &)> &&callback) {
    drogon::async_run([=]() -> Task<void> {
        co_await handleRequestAsync(req, std::move(callback));
    });
}