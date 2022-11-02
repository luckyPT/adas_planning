#include <iostream>
#include "src/osqp_test.hpp"
#include "src/easy_x_test.hpp"
#include <vector>
#include <string.h>
int main(int argc, char const *argv[])
{
    std::cout << "start running" << std::endl;
    std::cout << "run example osqp..." << std::endl;
    test_osqp();
    std::cout << "run example easy_x..." << std::endl;
    test_easy_x();
    return 0;
}
