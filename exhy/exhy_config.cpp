//
//  exhy_config.cpp
//  exhy
//
//  Created by Wxy on 2021/7/29.
//

#include "exhy_config.h"

namespace exhy{




//Config::ConfigVarMap Config::s_datas;
ConfigVarBase::ptr Config::LookupBase(const std::string& name){
    auto it = GetDatas().find(name);
    return it == GetDatas().end() ? nullptr : it->second;
}

static void ListAllMember(const std::string& prefix,const YAML::Node& node,
                          std::list<std::pair<std::string, const YAML::Node>>& output){
    if(prefix.find_first_not_of("abcdefghijklmnopqrstuvwxyz._0123456789")!=std::string::npos){
        EXHY_LOG_ERROR(EXHY_LOG_ROOT())<<"Config invalid name:"<<prefix<<":";
        return;
    }
    output.push_back(std::make_pair(prefix, node));
    if(node.IsMap()){
        for(auto it = node.begin();it!=node.end();++it){
            ListAllMember(prefix.empty()? it->first.Scalar():prefix+"."+it->first.Scalar(), it->second,output);
        }
    }
}
void Config::LoadFromYaml(const YAML::Node &root){
    std::list<std::pair<std::string,const YAML::Node> > all_nodes;
    ListAllMember("",root,all_nodes);
    
    for(auto& i : all_nodes){
        std::string key = i.first;
        std::cout<<"key =  "<<key<<std::endl;
        if(key.empty()) continue;
        std::transform(key.begin(), key.end(), key.begin(), ::tolower);
        ConfigVarBase::ptr var = LookupBase(key);
        if(var){
            if(i.second.IsScalar()){
                var->fromString(i.second.Scalar());
            }else{
                std::stringstream ss;
                ss<<i.second;
                var->fromString(ss.str());
            }
        }
        
    }
}




}

