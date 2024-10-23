#pragma once

#include <vector>
#include <string>

enum command {
    EMPTY_QUERY = 'I',
    SIMPLE_QUERY = 'Q',
    COMMAND_COMPLETE = 'C',
    EXECUTE = 'E'
};

class Parser
{
private:

    size_t decodeLength(const std::vector<uint8_t>& buf) {
        return (buf[1] << 5) + (buf[2] << 4) + (buf[3] << 3) + buf[4] - 4;
    }

public:
    std::string parse(std::vector <uint8_t> &buf);

};