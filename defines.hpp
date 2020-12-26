#pragma once

#define LOG_I std::cout
#define END_I std::endl

#define LOG_S LOG_I << "\033[92m"
#define END_S "\033[37m" << END_I

#define LOG_W LOG_I << "\033[33m"
#define END_W "\033[37m" << END_I

#define LOG_E LOG_I << "\033[31m"
#define END_E "\033[37m" << END_I