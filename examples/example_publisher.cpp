// System Includes
#include <string>
#include <vector>

// Project Includes
#include <spdlog/spdlog.h>
#include "redisClient.h"

// When a subsriber channel is triggered. This function is called.
void workerFunction(std::vector<std::string> message)
{
    // The vector of strings is each word seperated by the colon
    // For example
    //
    // Tiger:Eats:Fruit == {"Tiger", "Eats", "Fruit"}
    // Bird:Sings:Loudly == {"Bird", "Sings", "Loudly"}
    // Fish:Swims:Away == {"Fish", "Swims", "Away"}
    //
    // The naming convention is defined by the Publisher
}

int main(int argc, char** argv)
{
    std::string host_name = "localhost";
    int address_port = 6379;
    std::vector<std::string> subscriber_channels = {"Tiger", "Bird", "Fish"};

    spdlog::info("Hello World");

    // Create the Redis Client
    RedisClient client(host_name, address_port);

    // Set the channels to subscribe to
    client.setSubscriberChanels(subscriber_channels);

    // Bind the worker function to be called
    // Note if the function is a class member. You must also pass 'this'
    // setSubscriberCallback(std::bind(&Malfunctions::callback, this, std::placeholders::_1));
    client.setSubscriberCallback(std::bind(&workerFunction, std::placeholders::_1));

    // Set the Subscriber thread to listen to incoming messages
    client.startSubscriber();

    return 0;
};