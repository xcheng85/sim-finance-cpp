#include <iostream>
#include <memory>
#include <string>
#include "Poco/Thread.h"      //std::thread
#include "Poco/Runnable.h"    // functor, lamda, global function
#include "Poco/ThreadPool.h"  // no counterpart in c++ standard thread lib
#include "Poco/ThreadLocal.h" // thread local storage, c++ thread
#include "Poco/Redis/Client.h"
#include "Poco/Net/NetException.h"

class SimpleRunnable : public Poco::Runnable
{
    virtual void run()
    {
        std::cout << "SimpleRunnable" << std::endl;
    }
};

// Client(
//     const std::string & host,
//     int port
// );

int main()
{
    SimpleRunnable r;
    Poco::Thread t;
    t.start(r);
    t.join();

    // https://hub.docker.com/r/ubuntu/redis
    // run redis locally in docker
    // docker run -d --name redis-container -e TZ=UTC -p 30073:6379 -e REDIS_PASSWORD=mypassword ubuntu/redis:6.2-22.04_beta
    // Access your Redis server at localhost:30073

    // docker run -d --name redis-stack-server -p 6379:6379 redis/redis-stack-server:latest
    try
    {
        auto redisClient = std::make_unique<Poco::Redis::Client>(std::string("127.0.0.1"), 6479);
        std::cout << "redis connected: " << redisClient->isConnected() << std::endl;
        // check logs of container
        // docker logs --since=1h fb70a2a27182 -f
    }
    catch (const Poco::Net::ConnectionRefusedException &e)
    {
        std::cerr << e.what() << '\n';
    }

    return 0;
}
