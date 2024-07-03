#include "MyController.h"

void MyController::handleRequest(const HttpRequestPtr &req, std::function<void (const HttpResponsePtr &)> &&callback) {
    HttpViewData data;
    data.insert("title", "Hello, DROGON !");
    data.insert("number", "123");
    std::string r = "rezi";
    auto resp = HttpResponse::newHttpViewResponse("example.csp", data);
    callback(resp);
}