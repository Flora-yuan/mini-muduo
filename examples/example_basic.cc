#include "base/Logger.h"
#include "base/Timestamp.h"

#include <iostream>

int main()
{
    LOG_INFO("mini_muduo start");

    mini_muduo::Timestamp now = mini_muduo::Timestamp::now();
    std::cout << "current time: " << now.toString() << std::endl;
    std::cout << "formatted time: " << now.toFormattedString() << std::endl;

    return 0;
}
