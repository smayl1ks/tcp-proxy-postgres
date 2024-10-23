#include "../include/parser.hpp"

std::string Parser::parse(std::vector <uint8_t>& buf)
{
    std::string message = "";

    // | COMMAND |     LENGTH      | QUERY STRING
    // |  buf[0] | buf[1] - buf[4] | buf[5] ...
    if (buf[0] == SIMPLE_QUERY) {

        size_t length = decodeLength(buf);

        for (size_t i = 0; i < length; ++i)
            message.push_back(buf[5 + i]);
    }

    // if (buf[0] == COMMAND_COMPLETE) {
    //     message.push_back(COMMAND_COMPLETE);
    // }

    return message;
}