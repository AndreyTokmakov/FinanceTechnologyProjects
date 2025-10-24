/**============================================================================
Name        : main.cpp
Created on  : 23.10.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Common modules
============================================================================**/

#include <vector>
#include <iostream>
#include <string_view>
#include <print>
#include <thread>

// TODO:
//  -  Connector / Auth / Fetch data - Strategy
//  -  Parser - Strategey
//  -  RingBuffer


namespace data_feeder
{
    struct DataFeeder
    {
        template <typename Self>
        void run(this Self&& self)
        {
            while (true)
            {
                auto data = self.getData();
                auto parsed = self.parse(data);
                std::cout << parsed << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds (1U));
            }
        }

        std::string getData() {
            return "Default";
        }

        std::string parse(const std::string& str) {
            return "[" + str + "]";
        }
    };


    struct Feeder2: DataFeeder
    {
        std::string getData() {
            return "Feeder2-Data";
        }

        std::string parse(const std::string& str) {
            return "{" + str + "}";
        }
    };
}


int main([[maybe_unused]] int argc,
         [[maybe_unused]] char** argv)
{
    const std::vector<std::string_view> args(argv + 1, argv + argc);

    data_feeder::Feeder2 feeder;
    feeder.run();

    return EXIT_SUCCESS;
}
