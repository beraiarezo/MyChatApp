#include <drogon/HttpController.h>
#include <drogon/HttpAppFramework.h>
#include <drogon/drogon.h>
#include <jwt-cpp/jwt.h>
#include "RestfulUsersCtrlBase.h"
#include "Users.h" // Ensure this includes your Users class definition
#include "filters/AuthFilter.h"


using namespace drogon;
using namespace jwt;
using namespace drogon_model::chat; // Assuming your namespace is drogon_model::chat

class AuthCtrl : public drogon::HttpController<AuthCtrl>
{
public:
  METHOD_LIST_BEGIN
    ADD_METHOD_TO(AuthCtrl::login, "/login", Post, Options);
    // ADD_METHOD_TO(AuthCtrl::login, "/loginn", Post, "AuthFilter");
  METHOD_LIST_END

  

  void login(const HttpRequestPtr &req, std::function<void(const HttpResponsePtr &)> &&callback);

private:
    std::string createJwtToken(const Users &user);
    bool verifyJwtToken(const std::string &token);

};