/**
 *
 *  AuthFilter.cc
 *
 */

#include "AuthFilter.h"
#include <jwt-cpp/jwt.h>

using namespace drogon;

void AuthFilter::doFilter(const HttpRequestPtr &req,
                         FilterCallback &&fcb,
                         FilterChainCallback &&fccb)
{
    auto authHeader = req->getHeader("Authorization");
    if (authHeader.empty() || authHeader.find("Bearer ") != 0) {
        auto resp = HttpResponse::newHttpResponse();
        resp->setStatusCode(k401Unauthorized);
        fcb(resp);
        return;
    }

    auto token = authHeader.substr(7);
    try {
        auto decoded = jwt::decode(token);
        auto verifier = jwt::verify()
                            .allow_algorithm(jwt::algorithm::hs256{"secret"})
                            .with_issuer("chat_app");
        verifier.verify(decoded);
        fccb();
    } catch (const std::exception &e) {
        auto resp = HttpResponse::newHttpResponse();
        resp->setStatusCode(k401Unauthorized);
        fcb(resp);
    }
}

