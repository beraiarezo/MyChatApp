#include <drogon/drogon.h>
int main() {
    // auto dbClient = drogon::orm::DbClient::newPgClient(
    //     "host=127.0.0.1 port=5432 dbname=chat user=rezoberaia password=",
    // 1);

    //Set HTTP listener address and port
    drogon::app().addListener("localhost", 8000);
    //Load config file
    drogon::app().loadConfigFile("../config.json");
    //drogon::app().loadConfigFile("../config.yaml");
    //Run HTTP framework,the method will block in the internal event loop
    drogon::app().run();
    return 0;
}
