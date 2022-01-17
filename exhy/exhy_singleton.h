//
//  exhy_singleton.h
//  exhy
//
//  Created by Wxy on 2021/7/30.
//

#ifndef __EXHY_SINGLETON_H__
#define __EXHY_SINGLETON_H__

namespace exhy {

template<class T,class X = void, int N = 0>
class Singleton{
public:
    static T* GetInstance(){
        static T v;
        return &v;
    }
};

template<class T, class X = void, int N =0 >
class SingletonPtr{
public:
    static std::shared_ptr<T> GetInstance(){
        static std::shared_ptr<T> v(new T);
        return v;
    }
};

}



#endif /* exhy_singleton_h */















