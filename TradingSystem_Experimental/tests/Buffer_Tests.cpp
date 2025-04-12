/**============================================================================
Name        : BufferTest.cpp
Created on  : 12.04.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : BufferTest.cpp
============================================================================**/


#include <iostream>
#include <string_view>

#include "Buffer.h"
#include "Tests.h"


namespace {
    using namespace Common;

    void test_validateCapacity() {
        Buffer buffer;

        std::cout << buffer.size() << std::endl;
        buffer.incrementLength(1024);
        buffer.validateCapacity(512);

        std::cout << buffer.size() << std::endl;
    }

    void test_data()
    {
        Buffer buffer;

        std::string message{"qwerty"};
        {
            std::copy_n(message.data(), message.size(), buffer.head());
            buffer.incrementLength(message.length());
        }

        std::cout << '[' << std::string_view(buffer.data<char>(), buffer.size()) << "]\n";
    }


    void server_like_test()
    {
        Buffer buffer;
        std::string dataExpected;

        for (int i = 'a'; i < 'z'; ++i)
        {
            std::string message(128, (char)i);
            dataExpected += message;

            buffer.validateCapacity(128);
            std::copy_n(message.data(), message.size(), buffer.head());
            buffer.incrementLength(message.length());
        }

        std::string result(buffer.data<char>(), buffer.size());
        std::cout << std::boolalpha << (result == dataExpected) << std::endl;
    }
}


void Tests::BufferTests::TestAll()
{
    // test_validateCapacity();
    // test_data();
    server_like_test();
}