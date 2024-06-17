#include <drogon/drogon.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <drogon/HttpController.h>

using namespace drogon;


// Task<void> handleRequest(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback) {
//   // Use co_await to start asynchronous data fetching
//   auto data = co_await fetchData();

//   // Respond with the fetched data
//   auto response = HttpResponse::newHttpResponse();
//   response->setStatusCode(HttpStatusCode::k200OK);
//   response->setBody(data);

//   callback(response);
// }

void executeSchema(const std::vector<std::string>& sqlCommands) {
        LOG_DEBUG << "hellllooooiii";

    auto dbClient = drogon::app().getDbClient("default");
    if (!dbClient) {
        std::cerr << "Failed to get database client" << std::endl;
        return;
    }

    for (const auto& sqlCommand : sqlCommands) {
        dbClient->execSqlAsync(sqlCommand, [](const drogon::orm::Result &r) {
            LOG_INFO << "SQL command executed successfully.";
        }, [](const drogon::orm::DrogonDbException &e) {
            std::cerr << "Failed to execute SQL command: " << e.base().what() << std::endl;
        });
    }
}

Task<void> Handle1()
{
    std::cout << "Handle1" << std::endl;
    co_await sleepCoro(app().getLoop(), 10);
}
 
Task<void> Handle2()
{   
    co_await sleepCoro(app().getLoop(), 2);
    std::cout << "OLAAAAA" << std::endl;

}

int main() {
    try {
        drogon::app().loadConfigFile("../config.json");
        LOG_DEBUG << "Config file loaded successfully.";
    } catch (const std::exception &e) {
        std::cerr << "Failed to load config file: " << e.what() << std::endl;
        return 1;
    }

    std::ifstream schemaFile("../schema.sql");
    if (!schemaFile.is_open()) {
        std::cerr << "Failed to open schema.sql file" << std::endl;
        return 1;
    } else {
        LOG_DEBUG << "schema.sql file opened successfully.";
    }

    std::vector<std::string> sqlCommands;
    std::string sqlCommand;
    while (std::getline(schemaFile, sqlCommand, ';')) {
        if (!sqlCommand.empty()) {
            sqlCommands.push_back(sqlCommand);
        }
    }

    // Register an event to run after the HTTP server has started
    drogon::app().getLoop()->queueInLoop([sqlCommands]() {
        executeSchema(sqlCommands);
    });


    // Run the Drogon application
    drogon::app().run();

    return 0;
}