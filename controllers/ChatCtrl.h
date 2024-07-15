#pragma once

#include <drogon/HttpController.h>

using namespace drogon;
using namespace drogon::orm;


class ChatCtrl : public drogon::HttpController<ChatCtrl>
{
  public:
    METHOD_LIST_BEGIN
    // use METHOD_ADD to add your custom processing function here;
    METHOD_ADD(ChatCtrl::getChattedUsers, "/chatted_users?user_id={1}&limit={2}&offset={3}", Get);
    // METHOD_ADD(ChatCtrl::your_method_name, "/{1}/{2}/list", Get); // path is /ChatCtrl/{arg1}/{arg2}/list
    // ADD_METHOD_TO(ChatCtrl::your_method_name, "/absolute/path/{1}/{2}/list", Get); // path is /absolute/path/{arg1}/{arg2}/list

    METHOD_LIST_END
    void getChattedUsers(const HttpRequestPtr &req, std::function<void(const HttpResponsePtr &)> &&callback, const std::string &userId, int limit, int offset);

    // void get(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback, int p1, std::string p2);
    // void your_method_name(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback, double p1, int p2) const;
};
