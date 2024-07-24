#include "ChatCtrl.h"

// Add definition of your processing function here
void ChatCtrl::getChattedUsers(const HttpRequestPtr &req, std::function<void (const HttpResponsePtr &)> &&callback, const std::string &userId) {
        auto client = app().getDbClient();
        
        // Query to fetch paginated users
        std::string fetchQuery = R"(
            SELECT DISTINCT u.id, u.name, u.email, u.avatar
            FROM users u
            JOIN users_chats uc ON u.id = uc.user_id
            WHERE uc.chat_id IN (
                SELECT chat_id
                FROM users_chats
                WHERE user_id = $1
            )
            ORDER BY u.name
        )";

        // Query to count total users
        std::string countQuery = R"(
            SELECT COUNT(DISTINCT u.id)
            FROM users u
            JOIN users_chats uc ON u.id = uc.user_id
            WHERE uc.chat_id IN (
                SELECT chat_id
                FROM users_chats
                WHERE user_id = $1
            );
        )";

        // Execute the queries
        client->execSqlAsync(
            fetchQuery,
            [callback, client, countQuery, userId](const Result &r) {
                Json::Value users(Json::arrayValue);
                for (const auto &row : r) {
                    Json::Value user;
                    user["id"] = row["id"].as<std::string>();
                    user["name"] = row["name"].as<std::string>();
                    user["email"] = row["email"].as<std::string>();
                    user["avatar"] = row["avatar"].as<std::string>();
                    users.append(user);
                }

                auto resp = HttpResponse::newHttpJsonResponse(users);
                resp->addHeader("Access-Control-Allow-Origin", "*");
                callback(resp);
            },
            [callback](const DrogonDbException &e) {
                auto resp = HttpResponse::newHttpJsonResponse(Json::Value::nullSingleton());
                resp->setStatusCode(HttpStatusCode::k500InternalServerError);
                callback(resp);
            },
            userId
        );
    }