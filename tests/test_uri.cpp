#include "exhy/exhy_uri.h"
#include <iostream>
// "http://admin@www.wxy.com/wxy/lxh/中文测试uri/#ff"
int main(){
    std::string str;
    while(std::cin>>str){
        exhy::Uri::ptr uri = exhy::Uri::Create(str);
        std::cout<<uri->toString()<<std::endl;
        auto addr = uri->createAddress();
        std::cout<<"IP Address is============="<<std::endl;
        std::cout<<*addr<<std::endl;
    }

}