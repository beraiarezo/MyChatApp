/**
 *
 *  CorsFilter.cc
 *
 */

#include "CorsFilter.h"

using namespace drogon;

void CorsFilter::doFilter(const HttpRequestPtr &req,
                         FilterCallback &&fcb,
                         FilterChainCallback &&fccb)
{
    // auto res = drogon::HttpResponse::newHttpResponse();
    // res->addHeader("Access-Control-Allow-Origin", "*"); // or specify a specific origin
    // // res->addHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
    // // res->addHeader("Access-Control-Allow-Headers", "Content-Type, Authorization");
    // // res->addHeader("Content-Type", "application/json");

    //  if (req->method() == HttpMethod::Options) {
    //     fcb(res); // Respond to OPTIONS request immediately
    // } else {
    //     // Set content type and JSON response
    //     res->addHeader("Content-Type", "application/json");

    //     // Example JSON response
    //     // nlohmann::json jsonResponse = {
    //     //     {"message", "Hello, JSON!"}
    //     // };
    //     // res->setBody(jsonResponse.dump());

    //     fccb(); // Pass the response to the next filter or handler
    // }
}