#pragma once

#include <drogon/HttpController.h>

using namespace drogon;
using namespace drogon::orm;


class ChatCtrl : public drogon::HttpController<ChatCtrl>
{
  public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(ChatCtrl::getChattedUsers, "/chatted_users?user_id={1}", Get, Options);
    METHOD_LIST_END
    void getChattedUsers(const HttpRequestPtr &req, std::function<void(const HttpResponsePtr &)> &&callback, const std::string &userId);
};
