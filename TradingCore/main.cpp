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
void market_event_handler_test();
void execution_report_handler_test();
void book_builder_test();
void pnl_calculator_test();
void risk_manager_test();
void trade_recorder_test();
void position_test();
void position_manager_test();
void imbalance_strategy_test();
void strategy_executor_test();

// TODO:
//   Config
//   Metrics
//   Logging
//   CPU

/*
tests/
├── execution
│   ├── execution_report_handler_test.cpp
│   └── order_manager_test.cpp
│
├── integration
│   ├── market_data_to_strategy_test.cpp
│   └── order_execution_flow_test.cpp
│

*/

int main([[maybe_unused]] const int argc,
         [[maybe_unused]] char** argv)
{
    const std::vector<std::string_view> args(argv + 1, argv + argc);


    order_book_test();
    order_manager_test();
    market_event_handler_test();
    execution_report_handler_test();
    book_builder_test();
    pnl_calculator_test();
    risk_manager_test();
    trade_recorder_test();
    position_test();
    position_manager_test();
    imbalance_strategy_test();
    strategy_executor_test();

    return EXIT_SUCCESS;
}
