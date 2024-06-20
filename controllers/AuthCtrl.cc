#include "AuthCtrl.h"
#include <bcrypt/BCrypt.hpp> 


void AuthCtrl::login(const HttpRequestPtr &req, std::function<void(const HttpResponsePtr &)> &&callback)
{
    // Extract credentials from the request
    auto jsonPtr = req->getJsonObject();
    if (!jsonPtr || !jsonPtr->isMember("email") || !jsonPtr->isMember("password_hash"))
    {
        auto resp = HttpResponse::newHttpJsonResponse(Json::Value("Invalid credentials"));
        resp->setStatusCode(k400BadRequest);
        callback(resp);
        return;
    }

    std::string email = (*jsonPtr)["email"].asString();
    std::string password = (*jsonPtr)["password_hash"].asString();

    auto dbClientPtr = app().getDbClient();
    drogon::orm::Mapper<Users> mapper(dbClientPtr);

    mapper.findBy(drogon::orm::Criteria(Users::Cols::_email, drogon::orm::CompareOperator::EQ, email),
        [req, email, password, callback, this](const std::vector<Users> &users) {
            if (users.empty())
            {
                auto resp = HttpResponse::newHttpJsonResponse(Json::Value("User not found"));
                resp->setStatusCode(k404NotFound);
                callback(resp);
                return;
            }

            const auto &user = users.front();
            std::string storedPasswordHash = user.getValueOfPasswordHash();

            
            if (!BCrypt::validatePassword(password, storedPasswordHash)) // Verify password
            {
                auto resp = HttpResponse::newHttpJsonResponse(Json::Value("Invalid credentials"));
                resp->setStatusCode(k401Unauthorized);
                callback(resp);
                return;
            }

            // Generate JWT token
            auto token = createJwtToken(user);

            Json::Value ret;
            ret["token"] = token;
            auto resp = HttpResponse::newHttpJsonResponse(ret);
            callback(resp);
        },
        [callback](const DrogonDbException &e) {
            LOG_ERROR << e.base().what();
            Json::Value ret;
            ret["error"] = "database error";
            auto resp = HttpResponse::newHttpJsonResponse(ret);
            resp->setStatusCode(k500InternalServerError);
            callback(resp);
        });
}

std::string AuthCtrl::createJwtToken(const Users &user)
{
    // Create the JWT token with user details
    auto token = create()
                    .set_issuer("chat_app")
                    .set_type("JWT")
                    .set_payload_claim("username", claim(std::string(user.getValueOfName())))
                    // Add other claims as needed
                    .sign(jwt::algorithm::hs256{"secret"});
    return token;
}

