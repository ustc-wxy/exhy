#include <iostream>
#include <thread>
#include <yaml-cpp/yaml.h>

#include "exhy_log.h"
#include "exhy_util.h"
#include "exhy_config.h"
#include "exhy_thread.h"
#include "exhy_fiber.h"

using namespace std;
class Person {
public:
    Person() {};
    std::string name;
    std::string m_name;
    int m_age = 0;
    bool m_sex = 0;

    std::string toString() const {
        std::stringstream ss;
        ss << "[Person name=" << m_name
           << " age=" << m_age
           << " sex=" << m_sex
           << "]";
        return ss.str();
    }

    bool operator==(const Person& oth) const {
        return m_name == oth.m_name
            && m_age == oth.m_age
            && m_sex == oth.m_sex;
    }
    bool operator<(const Person& oth) const {
        return m_name < oth.m_name;
    }
};
namespace exhy {

template<>
class LexicalCast<std::string, Person> {
public:
    Person operator()(const std::string& v) {
        YAML::Node node = YAML::Load(v);
        Person p;
        p.m_name = node["name"].as<std::string>();
        p.m_age = node["age"].as<int>();
        p.m_sex = node["sex"].as<bool>();
        return p;
    }
};

template<>
class LexicalCast<Person, std::string> {
public:
    std::string operator()(const Person& p) {
        YAML::Node node;
        node["name"] = p.m_name;
        node["age"] = p.m_age;
        node["sex"] = p.m_sex;
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

}

exhy::ConfigVar<Person>::ptr g_person = exhy::Config::Lookup("class.person", Person(), "system person");
//exhy::ConfigVar<std::set<Person> >::ptr g_person_set = exhy::Config::Lookup("class.person_set", std::set<Person>(), "system person set");
exhy::ConfigVar<int>::ptr g_int_value_config = exhy::Config::Lookup("system.port", (int)8080, "system port");
//test commn name
//exhy::ConfigVar<float>::ptr g_int_valuex_config = exhy::Config::Lookup("system.port", (float)8080, "system port");
//test commn name
exhy::ConfigVar<float>::ptr g_float_value_config = exhy::Config::Lookup("system.value", (float)8.3f,"system value");
exhy::ConfigVar<std::vector<int> >::ptr g_int_vec_value_config = exhy::Config::Lookup("system.int_vec", std::vector<int>{1,2},"system int vec");
exhy::ConfigVar<std::list<int> >::ptr g_int_list_value_config = exhy::Config::Lookup("system.int_list", std::list<int>{1,2},"system int list");
exhy::ConfigVar<std::set<int> >::ptr g_int_set_value_config = exhy::Config::Lookup("system.int_set", std::set<int>{1,2},"system int set");
exhy::ConfigVar<std::unordered_set<int> >::ptr g_int_uset_value_config = exhy::Config::Lookup("system.int_uset", std::unordered_set<int>{9,7},"system int uset");
exhy::ConfigVar<std::map<std::string,int> >::ptr g_str_int_map_value_config = exhy::Config::Lookup("system.int_map", std::map<std::string,int>{{"E",1}},"system str int map");
exhy::ConfigVar<std::unordered_map<std::string,int> >::ptr g_str_int_unordered_map_value_config = exhy::Config::Lookup("system.int_unordered_map", std::unordered_map<std::string,int>{{"L",5}},"system str int unordered map");


void print_yaml(const YAML::Node& node, int level){
    if(node.IsScalar()){
        EXHY_LOG_INFO(EXHY_LOG_ROOT())<<std::string(level*4,' ')<< node.Scalar() << "-" << node.Type() << "-" <<level;
    }else if(node.IsNull()){
        EXHY_LOG_INFO(EXHY_LOG_ROOT())<<std::string(level*4,' ') << node.Scalar() << "-" << node.Type() << "-" <<level;
    }else if(node.IsMap()){
        for(auto it = node.begin();it!=node.end();++it){
            EXHY_LOG_INFO(EXHY_LOG_ROOT())<<std::string(level*4,' ')<<it->first<<"-"<<it->second.Type()<<"-"<<level;
            print_yaml(it->second, level + 1);
        }
    }else if(node.IsSequence()){
        for(size_t i = 0;i < node.size(); i++){
            EXHY_LOG_INFO(EXHY_LOG_ROOT())<<std::string(level*4,' ')<<i<<"-"<<node[i].Type()<<"-"<<level;
            print_yaml(node[i], level+1);
        }
    }
}
void test_yaml(){
    
    YAML::Node root = YAML::LoadFile("/Users/wxy/Documents/exhy_prj/exhy/exhy/log.yml");
    //EXHY_LOG_INFO(EXHY_LOG_ROOT())<<root;
    print_yaml(root, 0);
}
void test_config(){
    EXHY_LOG_INFO(EXHY_LOG_ROOT())<<"before: "<<g_int_value_config->getValue();
    EXHY_LOG_ERROR(EXHY_LOG_ROOT())<<"before: "<<g_float_value_config->getValue();
#define PT(g_var,name,prefix) \
    {\
        auto v = g_var->getValue(); \
        for(auto& i : v){ \
            EXHY_LOG_INFO(EXHY_LOG_ROOT())<< #prefix " " #name ": "<<i; \
        }\
        EXHY_LOG_INFO(EXHY_LOG_ROOT())<< #prefix " " #name " yaml: "<<g_var->toString(); \
    }
#define PT_M(g_var,name,prefix) \
    {\
        auto v = g_var->getValue(); \
        for(auto& i : v){ \
            EXHY_LOG_INFO(EXHY_LOG_ROOT())<< #prefix " " #name ": {"<<i.first<<" - "<<i.second<<"}"; \
        }\
        EXHY_LOG_INFO(EXHY_LOG_ROOT())<< #prefix " " #name " yaml: "<<g_var->toString(); \
    }
    PT(g_int_vec_value_config,int_vec,before);
    PT(g_int_list_value_config,int_list,before);
    PT(g_int_set_value_config,int_set,before);
    PT(g_int_uset_value_config,int_uset,before);
    PT_M(g_str_int_map_value_config,int_map,before);
    PT_M(g_str_int_unordered_map_value_config,int_map,before);

    YAML::Node root = YAML::LoadFile("/Users/wxy/Documents/exhy_prj/exhy/exhy/log.yml");
    exhy::Config::LoadFromYaml(root);
    EXHY_LOG_INFO(EXHY_LOG_ROOT())<<"after: "<<g_int_value_config->getValue();
    EXHY_LOG_ERROR(EXHY_LOG_ROOT())<<"after: "<<g_float_value_config->getValue();
    PT(g_int_vec_value_config,int_vec,after);
    PT(g_int_list_value_config,int_list,after);
    PT(g_int_set_value_config,int_set,after);
    PT(g_int_uset_value_config,int_uset,after);
    PT_M(g_str_int_map_value_config,int_map,after);
    PT_M(g_str_int_unordered_map_value_config,int_map,after);
}
void test_class(){
    EXHY_LOG_INFO(EXHY_LOG_ROOT())<<" before: "<<g_person->getValue().toString()<<" - "<<g_person->toString();
    g_person->addListener([](const Person& old_value, const Person& new_value){
                EXHY_LOG_INFO(EXHY_LOG_ROOT()) << "old_value=" << old_value.toString()
                        << " new_value=" << new_value.toString();
            });
    YAML::Node root = YAML::LoadFile("/Users/wxy/Documents/exhy_prj/exhy/exhy/log.yml");
    exhy::Config::LoadFromYaml(root);
    EXHY_LOG_INFO(EXHY_LOG_ROOT())<<" after: "<<g_person->getValue().toString()<<" - "<<g_person->toString();
}
void test_log(){
    std::cout<<exhy::LoggerMgr::GetInstance()->toYamlString()<<std::endl;
    YAML::Node root = YAML::LoadFile("/home/wxy/exhy/log.yml");
    exhy::Config::LoadFromYaml(root);
    std::cout<<"====================="<<std::endl;
    std::cout<<exhy::LoggerMgr::GetInstance()->toYamlString()<<std::endl;
    static exhy::Logger::ptr system_log = EXHY_LOG_NAME("system");
    EXHY_LOG_INFO(system_log)<<"hello system"<<std::endl;
}
int main()
{
    std::cout<<"Hello exhy log "<<exhy::GetThreadId()<<std::endl;
    //std::cout << "Boost版本：" << BOOST_VERSION << std::endl;
    //test_yaml();
    //test_config();
    //test_class();
    test_log();
    return 0;
}
