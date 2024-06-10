#include <drogon/drogon.h>
#include <fstream>
#include <sstream>
#include <iostream>

void executeSchema(const std::vector<std::string>& sqlCommands) {
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