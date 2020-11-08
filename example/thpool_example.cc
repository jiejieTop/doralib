/*
 * @Author: jiejie
 * @GitHub: https://github.com/jiejieTop
 * @Date: 2020-10-26 13:26:44
 * @LastEditors: jiejie
 * @LastEditTime: 2020-11-08 11:10:19
 * @Description: the code belongs to jiejie, please keep the author information and source code according to the license.
 */

#include <iostream>

#include "dora_thpool.h"
#include "dora_log.h"


// Simple function that adds multiplies two numbers and prints the result
void multiply(const int a, const int b) 
{
    const int res = a * b;
    DORA_LOG_INFO("{} * {} = {}", a, b, res);
}

// Same as before but now we have an output parameter
void multiply_output(int & out, const int a, const int b) 
{
    out = a * b;
    DORA_LOG_INFO("{} * {} = {}", a, b, out);
}

// Same as before but now we have an output parameter
int multiply_return(const int a, const int b) 
{
    const int res = a * b;
    DORA_LOG_INFO("{} * {} = {}", a, b, res);
    return res;
}

int main()
{
    DORA_LOG_INFO("========================= thpool =========================");
    
    DORA_LOG_SET_DEBUG_LEVEL
    
    auto pool = new doralib::thpool;

    // Submit (partial) multiplication table
    for (int i = 1; i < 3; ++i) {
        for (int j = 1; j < 10; ++j) {
            pool->submit(multiply, i, j);
        }
    }

    // Submit function with output parameter passed by ref
    int output_ref;
    auto future1 = pool->submit(multiply_output, std::ref(output_ref), 5, 6);

    // Wait for multiplication output to finish
    future1.get();
    DORA_LOG_INFO("last operation result is equals to {}", output_ref);

    // Submit function with return parameter 
    auto future2 = pool->submit(multiply_return, 5, 3);

    // Wait for multiplication output to finish
    int res = future2.get();
    DORA_LOG_INFO("last operation result is equals to {}", res);

    pool->shutdown();

    DORA_LOG_INFO("========================= success =========================");

    return 0;
}

