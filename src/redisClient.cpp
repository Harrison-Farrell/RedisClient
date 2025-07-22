#include "redisClient.h"

// System Includes
#include <stdexcept>
#include <string.h>
#include <cstring>
#include <sstream>

RedisClient::RedisClient(std::string host, int port)
    : serverHost(host), serverPort(port), publisher(nullptr), subscriber(nullptr)
{
    connect();
}

RedisClient::~RedisClient()
{
    if (publisher) {
        redisFree(publisher.get());
    }
    if (subscriber) {
        redisFree(subscriber.get());
    }
}

void RedisClient::setSubscriberChanels(std::vector<std::string> channels)
{
    subscriberChannels = channels;
}

std::vector<std::string> RedisClient::getSubscriberChannels() const
{
    return subscriberChannels;
}

void RedisClient::connect()
{
    // Create a new Redis context for the publisher
    publisher.reset(redisConnect(serverHost.c_str(), serverPort));
    if (publisher == nullptr || publisher->err) {
        throw std::runtime_error("Could not connect to Redis publisher");
    }

    // Create a new Redis context for the subscriber
    subscriber.reset(redisConnect(serverHost.c_str(), serverPort));
    if (subscriber == nullptr || subscriber->err) {
        throw std::runtime_error("Could not connect to Redis subscriber");
    }
}

void RedisClient::startSubscriber()
{
    if (subscriberRunning.load()) {
        return; // Already running
    }
    subscriberRunning.store(true);
    // Start the subscriber loop in a separate thread
    subscriberThread = std::thread(&RedisClient::subscriberLoop, this);
}

void RedisClient::stopSubscriber()
{
    if (!subscriberRunning.load()) {
        return; // Not running
    }
    subscriberRunning.store(false);
    if (subscriberThread.joinable()) {
        subscriberThread.join();
    }
}

void RedisClient::subscriberLoop()
{
    redisReply* reply = nullptr;
    for(auto& channel : subscriberChannels)
    {
        reply = static_cast<redisReply*>(redisCommand(subscriber.get(), "SUBSCRIBE %s",channel.c_str()));
        freeReplyObject(reply);
    }

    while (subscriberRunning)
    {
        if (redisGetReply(subscriber.get(), (void**)&reply) == REDIS_OK)
        {
            if (reply && reply->type == REDIS_REPLY_ARRAY && reply->elements == 3) {
                // Process the message
                messageHandler(splitString(reply->element[2]->str));
            } else {
                freeReplyObject(reply);
            }
        } else
        {
            throw std::runtime_error("Failed to receive reply from Redis");
            break;
        }
    }
}

void RedisClient::publish(const std::string& channel, const std::string& message)
{
    if (!publisher) {
        throw std::runtime_error("Publisher not initialized");
    }
    redisCommand(publisher.get(), "PUBLISH %s %s", channel.c_str(), message.c_str());
}

void RedisClient::setAndPublish(const std::string& channel, const std::string& message)
{
    if (!publisher) {
        throw std::runtime_error("Publisher not initialized");
    }

    std::string joint = channel + ":" + message;
    redisReply* reply = static_cast<redisReply*>(redisCommand(publisher.get(), "PUBLISH %s %s", channel.c_str(), joint.c_str()));
    freeReplyObject(reply);

    // Restructing the channel and message
    // CHANNEL | SECTION:SECTION:VALUE
    // CHANNEL:SECTION:SECTION | VALUE
    std::vector<std::string> channel_sections = splitString(message.c_str());
    std::string value = channel_sections.back();
    channel_sections.pop_back();
    channel_sections.insert(channel_sections.begin(), channel);


    reply = static_cast<redisReply*>(redisCommand(publisher.get(), "SET %s %s", joinStrings(channel_sections, ":").c_str(), value.c_str()));
    freeReplyObject(reply);
}

void RedisClient::setSubscriberCallback(std::function<void(std::vector<std::string>)> callback)
{
    messageHandler = callback;
}

redisReply* RedisClient::executeCommand(std::string command) const
{
    if (!publisher) {
        throw std::runtime_error("Context not initialized");
    }
    redisReply* reply = static_cast<redisReply*>(redisCommand(publisher.get(), command.c_str()));
    if (reply == nullptr) {
        throw std::runtime_error("Failed to execute command");
    }

    // Return the response
    return reply;
}

bool RedisClient::getButtonState(std::string key) const
{
    bool state = false;
    std::string message = "GET ";
    message += key;

    redisReply* response = executeCommand(message.c_str());

    if (response->type == REDIS_REPLY_STRING)
    {
        if(strcmp(response->str, "ON") == 0)
        {
            // strcmp returns 0 when the strings are equal
            state = true; // The button is ON
        }
        else
        {
            state = false; // The button is OFF
        }
    }
    else
    {
        // The key does not exist or the response is not in the expected format
        // Creating the key and setting to the off state
        message = "SET " + key + " OFF";
        executeCommand(message.c_str());
    }

    freeReplyObject(response);
    return state;
}

bool RedisClient::getState(std::string value)
{
    if(value == "ON")
    {
        return true;
    }
    return false;
}

std::vector<std::string> RedisClient::splitString(const char* s) {
    std::vector<std::string> tokens; // Vector to store the resulting substrings
    std::string token;               // Temporary string to hold each token
    std::stringstream ss(s);         // Create a stringstream from the input string

    // Use std::getline to read from the stringstream until the delimiter is found
    // Each read segment is stored in 'token'
    while (std::getline(ss, token, ':')) {
        tokens.push_back(token); // Add the extracted token to the vector
    }

    return tokens; // Return the vector of substrings
}

std::string RedisClient::joinStrings(const std::vector<std::string>& strings, const std::string& separator)
const {
    if (strings.empty()) {
        return ""; // Return an empty string if the input vector is empty
    }

    std::string result;
    // Iterate through all but the last element, appending string and separator
    for (size_t i = 0; i < strings.size() - 1; ++i) {
        result += strings[i];
        result += separator;
    }
    // Append the last element (without a trailing separator)
    result += strings.back();

    return result; // Return the concatenated string
}


