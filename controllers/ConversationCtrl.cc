#include "ConversationCtrl.h"
#include "EchoWebsock.h"
#include <coroutine>
#include <shared_mutex>
#include <mutex>

using mutex_type = std::shared_mutex;
using read_only_lock = std::shared_lock<mutex_type>;
using updatable_lock = std::unique_lock<mutex_type>;

using namespace drogon;

// Function to notify user via WebSocket
// void notifyUser(const std::string &userId, const std::string &message) {
//     auto &wsController = EchoWebsock::getInstance();
//     for (const auto &row : participantsResult) {
//         std::string userId = row["user_id"].as<std::string>();
//         wsController.sendMessageToUser(userId, "You have received a new message in conversation " + conversationId);
//     }

    
// }

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


Task<void> ConversationCtrl::performTransaction(
    const std::string &conversationName,
    bool isGroup,
    const std::vector<std::string> &userIds,
    const std::string &messageContent,
    const std::string &senderId,
    const std::shared_ptr<drogon::orm::DbClient> &dbClient) {
    // Begin transaction
    co_await dbClient->execSqlCoro("BEGIN");

    // Step 1: Create Conversation
    auto chatRow = co_await createChat(senderId);
    std::string chatId = chatRow["id"].as<std::string>();

    // Step 2: Add Participants
    co_await addParticipants(chatId, userIds);

    // Step 3: Add a Message to Conversation
    auto messageRow = co_await addMessage(chatId, senderId, messageContent);
    std::string messageId = messageRow["id"].as<std::string>();

    // Link Message to Conversation

    // Notify users about the new message
    // auto participantsResult = co_await dbClient->execSqlCoro("SELECT user_id FROM participants WHERE conversation_id = $1", conversationId);
    // auto wsController = EchoWebsock;
    echoWebsock_->sendMessageToUser(userIds, messageContent);
    // wsController->sendMessageToUser("8681f5e9-e2fd-434e-9ca2-b774cd39fbcd", "You have received a new message in conversation ");
    // wsController.get()->sendMessageToUser()
    // for (const auto &row : participantsResult) {
    //     std::string userId = row["user_id"].as<std::string>();
    //     wsController.sendMessageToUser(userId, "You have received a new message in conversation ");
    // }

    // Commit transaction
    co_await dbClient->execSqlCoro("COMMIT");
}

Task<void> ConversationCtrl::createConversationWithParticipantsAndMessages(
    const std::string &conversationName,
    bool isGroup,
    const std::vector<std::string> &userIds,
    const std::string &messageContent,
    const std::string &senderId) {
    auto dbClient = app().getDbClient();
    bool exceptionOccurred = false;

    try {
        // Perform the transaction
        co_await performTransaction(conversationName, isGroup, userIds, messageContent, senderId, dbClient);
    } catch (const orm::DrogonDbException &e) {
        LOG_ERROR << "Error in transaction: " << e.base().what();
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
    std::string conversationName = (*json)["conversationName"].asString();
    bool isGroup = (*json)["isGroup"].asBool();
    auto userIdsJson = (*json)["userIds"];
    std::vector<std::string> userIds;
    for (const auto &userId : userIdsJson) {
        userIds.push_back(userId.asString());
    }
    std::string messageContent = (*json)["messageContent"].asString();
    std::string senderId = (*json)["senderId"].asString();

    try {
        // Perform the transaction
        co_await createConversationWithParticipantsAndMessages(conversationName, isGroup, userIds, messageContent, senderId);
        auto resp = HttpResponse::newHttpResponse();
        resp->setBody("Conversation created successfully!");
        callback(resp);
    } catch (const std::exception &e) {
        auto resp = HttpResponse::newHttpResponse();
        resp->setStatusCode(HttpStatusCode::k500InternalServerError);
        resp->setBody("Error creating conversation");
        callback(resp);
    }
}

void ConversationCtrl::handler(const HttpRequestPtr &req, std::function<void(const HttpResponsePtr &)> &&callback) {
    // Launch the coroutine
    // handleRequestAsync(req, std::move(callback));

    drogon::async_run([=]() -> Task<void> {
        // auto dbClient = app().getDbClient();

        //  co_await dbClient->execSqlCoro("INSERT INTO conversations (name, is_group) VALUES ($1, $2) RETURNING id", "name", false);
           co_await handleRequestAsync(req, std::move(callback));

        // auto sql = app().getDbClient();

        // auto result = co_await sql->execSqlCoro("SELECT COUNT(*) FROM users;");
        // size_t num_users = result[0][0].as<size_t>();
        // // auto resp = HttpResponse::newHttpResponse();
        // // resp->setBody(std::to_string(num_users));

        // co_await sleepCoro(app().getLoop(), 1);

        // std::cout << "Done" << std::endl;

        // // co_return 5;
         
           
        // co_await sleepCoro(app().getLoop(), 1);
        // co_await Handle2();
        // std::cout << "Done11111111" << std::endl;
    });
}