#pragma once

#include <drogon/HttpController.h>

using namespace drogon;

class MyController : public drogon::HttpController<MyController> {
    public:
        METHOD_LIST_BEGIN
        ADD_METHOD_TO(MyController::handleRequest, "/example", Get);
        METHOD_LIST_END

    void handleRequest(const HttpRequestPtr &req, std::function<void(const HttpResponsePtr &)> &&callback);

};