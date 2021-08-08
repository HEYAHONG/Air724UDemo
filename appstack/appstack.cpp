
#include "appstack.hpp"
#include "vector"

void app_init()
{

    {
        //测试new 与 delete
        std::vector<int> v;
        v.push_back(1);
    }
}

bool app_loop()
{
    return true;
}

void app_exit()
{

}
