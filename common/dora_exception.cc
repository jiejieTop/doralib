/*
 * @Author: jiejie
 * @Github: https://github.com/jiejieTop
 * @Date: 2019-12-09 21:31:25
 * @LastEditTime: 2020-10-28 15:26:45
 * @Description: the code belongs to jiejie, please keep the author information and source code according to the license.
 */
#include <errno.h>
#include <stdio.h>
#include <cstring>
#include <sstream>
#include <string>

/**
 * @file exception.cpp
 * @brief Exception class in libsocket **CLASS FOR INTERNAL USE ONLY**
 *
 * The class defined here, exception, is a
 * class for exception objects. Objects instantiated
 * from this class are thrown if something goes wrong.
 *
 * The class contains only the data member mesg and the
 * constructor which constructs the error string in mesg.
 * Typically, you'd create an object of this class with the
 * following call: `exception(__FILE__,__LINE__,"Error Message");`
 *
 * The tokens are substituded by the preprocessor and show where
 * the error occurred.
 */

#include <dora_exception.h>
#include <spdlog/spdlog.h>

namespace doralib {
using std::string;

/**
 * @brief	Constructor of a exception object
 *
 * This constructor creates a new exception object.
 *
 * @param	f   File in which the error comes (__FILE__)
 * @param	l   Line (__LINE__)
 * @param	m   Description of the error.
 */
void exception(const string& file, int line, const string& message, bool show_errno = true)
{
    std::ostringstream message_stream;

    message_stream << file << ":" << line << ": " << message;
    if (show_errno) message_stream << " errnor : (" << std::strerror(errno) << ")";

    DORA_LOG_ERROR("{}", message_stream.str());
}

void exception(char *file, int line, char *message, bool show_errno = true) 
{
    std::ostringstream message_stream;

    message_stream << file << ":" << line << ": " << message;
    if (show_errno) message_stream << " errnor : (" << std::strerror(errno) << ")";

    DORA_LOG_ERROR("{}", message_stream.str());
}

}
