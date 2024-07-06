#pragma once

#include <drogon/HttpController.h>
#include "filters/AuthFilter.h"
#include "EchoWebsock.h"
#include <drogon/drogon_callbacks.h>
// #include <drogon/HttpAppFramework.h>

using namespace drogon;

class ConversationCtrl : public drogon::HttpController<ConversationCtrl, false>
{
  public:

    ConversationCtrl(std::shared_ptr<EchoWebsock> echoWebsock)
        : echoWebsock_(echoWebsock) {}

    void asyncHandleHttpRequest(const std::shared_ptr<HttpRequest> &req,
                                      std::function<void(const std::shared_ptr<HttpResponse>)> &&callback);
    METHOD_LIST_BEGIN
    // // list path definitions here;
    // // PATH_ADD("/createConversation", "AuthFilter", Post);
    ADD_METHOD_TO(ConversationCtrl::handler, "/createConversation", Post);

    METHOD_LIST_END

    // METHOD_HANDLER(Post, "/createConversation")

    Task<void> performTransaction(
      const std::string &conversationName,
      bool isGroup,
      const std::vector<std::string> &userIds,
      const std::string &messageContent,
      const std::string &senderId,
      const std::shared_ptr<drogon::orm::DbClient> &dbClient);

    Task<void> createConversationWithParticipantsAndMessages(
      const std::string &conversationName,
      bool isGroup,
      const std::vector<std::string> &userIds,
      const std::string &messageContent,
      const std::string &senderId);

    Task<void> handleRequestAsync(const HttpRequestPtr &req, std::function<void(const HttpResponsePtr &)> callback);

    // asyncHandleHttpRequest(const HttpRequestPtr &req, std::function<void(const HttpResponsePtr &)> &&callback);
    void handler(const HttpRequestPtr &req, std::function<void(const HttpResponsePtr &)> &&callback);

    private:
      std::shared_ptr<EchoWebsock> echoWebsock_;
};

