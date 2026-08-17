/**============================================================================
Name        : main.cpp
Created on  : 24.28.2024
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Tests C++ project
============================================================================**/

#include <iostream>
#include <vector>
#include <string_view>


void order_book_test();
void order_manager_test();
void book_builder_test();

int main([[maybe_unused]] const int argc,
         [[maybe_unused]] char** argv)
{
    const std::vector<std::string_view> args(argv + 1, argv + argc);

    order_book_test();
    order_manager_test();
    book_builder_test();

    return EXIT_SUCCESS;
}
