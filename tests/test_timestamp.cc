#include "base/Logger.h"
#include "base/Timestamp.h"

#include <iostream>

int main()
{
    LOG_INFO("test_timestamp start");

    mini_muduo::Timestamp now = mini_muduo::Timestamp::now();
    std::cout << "microSecondsSinceEpoch: " << now.microSecondsSinceEpoch() << std::endl;
    std::cout << "secondsSinceEpoch: " << now.secondsSinceEpoch() << std::endl;
    std::cout << "toString: " << now.toString() << std::endl;
    std::cout << "toFormattedString: " << now.toFormattedString() << std::endl;

    LOG_INFO("test_timestamp end");
    return 0;
}
