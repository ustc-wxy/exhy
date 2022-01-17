#include "exhy/exhy.h"
#include <map>
#include <vector>
exhy::Logger::ptr g_logger = EXHY_LOG_ROOT();

void test(){
    std::vector<exhy::Address::ptr> addrs;
    bool v = exhy::Address::Lookup(addrs,"www.baidu.com:ftp");
    if(!v){
        EXHY_LOG_ERROR(g_logger)<<"lookup fail";
        return;
    }
    for(size_t i=0;i<addrs.size();i++){
        EXHY_LOG_INFO(g_logger)<<i<<"-"<<addrs[i]->toString();
    }
}
void test_iface(){
    std::multimap<std::string,std::pair<exhy::Address::ptr, uint32_t> > results;
    bool v = exhy::Address::GetInterfaceAddresses(results);
    if(!v){
        EXHY_LOG_ERROR(g_logger)<<"getInterface fail";
        return;
    }
    for(auto& i: results){
        EXHY_LOG_INFO(g_logger)<<i.first<<"-"<<i.second.first->toString()<<"-"
            <<i.second.second;
    }
}
void test_ipv4(){
    auto addr = exhy::IPAddress::Create("127.0.0.8");
    if(addr){
        EXHY_LOG_INFO(g_logger)<<addr->toString();
    }
}


int main(){
    test_ipv4();
    return 0;
}
