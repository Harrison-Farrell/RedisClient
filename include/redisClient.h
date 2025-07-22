#ifndef REDIS_CLIENT_H
#define REDIS_CLIENT_H

// System Includes
#include <memory>
#include <string>
#include <thread>
#include <atomic>
#include <vector>
#include <functional>

// Project Includes
#include <hiredis.h>

class RedisClient
{

public:

    RedisClient(std::string host, int port);
    ~RedisClient();

    // Context methods
    redisReply* executeCommand(std::string command) const;
    bool getButtonState(std::string key) const;

    // Publisher methods
    void publish(const std::string& channel, const std::string& message);
    void setAndPublish(const std::string& channel, const std::string& message);

    // Subscriber methods
    void startSubscriber();
    void stopSubscriber();
    void setSubscriberCallback(std::function<void(std::vector<std::string>)> callback);
    void setSubscriberChanels(std::vector<std::string> channels);
    std::vector<std::string> getSubscriberChannels() const;

    // Help methods
    static std::string joinStrings(const std::vector<std::string>& strings, const std::string& separator);
    bool getState(std::string value);

private:
    // member variables
    const std::string serverHost;
    const int serverPort;
    std::atomic<bool> subscriberRunning{false};
    std::vector<std::string> subscriberChannels;
    std::function<void(std::vector<std::string>)> messageHandler;

    // Redis connection parameters
    std::shared_ptr<redisContext> publisher= nullptr;
    std::shared_ptr<redisContext> subscriber= nullptr;

    // Threads for subscriber and publisher
    std::thread subscriberThread;
    std::thread publisherThread;

    // Helper functions
    void connect();
    void disconnect();
    void subscriberLoop();
    std::vector<std::string> splitString(const char* s);

};

#endif // REDIS_CLIENT_H
