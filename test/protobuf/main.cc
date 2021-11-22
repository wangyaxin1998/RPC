#include "test.pb.h"
#include <iostream>
#include <string>
using namespace fixbug;
int main()
{
#if 0
    //封装了login请求对象的数据
    LoginRequest req;
    req.set_name("zhang san");
    req.set_pwd("123456");
    //对象数据序列化——>char*
    std::string send_str;
    if(req.SerializePartialToString(&send_str)){
        std::cout<<send_str.c_str()<<std::endl;
    }

    //从send_str反序列化一个login请求对象
    LoginRequest reqB;
    if(reqB.ParseFromString(send_str))
    {
        std::cout<<reqB.name()<<std::endl;
        std::cout<<reqB.pwd()<<std::endl;
    }

    LoginResponse rsp;
    ResultCode *rc=rsp.mutable_result();
    rc->set_errorcode(1);
    rc->set_errmsg("登录处理失败了");
#endif
    GetFriednListsResponse rsp;
    //对于对象类成员指针的获取
    ResultCode *rc = rsp.mutable_result();
    rc->set_errorcode(0);

    //对于列表成员的条件
    User *user1 = rsp.add_friend_list();
    user1->set_name("zhang san");
    user1->set_age(23);
    user1->set_sex(User::MAN);

    User *user2 = rsp.add_friend_list();
    user2->set_name("lisi");
    user2->set_age(20);
    user2->set_sex(User::MAN);

    std::cout << rsp.friend_list_size() << std::endl;
    
    //序列化
    std::string send_str;
    if(rsp.SerializePartialToString(&send_str))
    {
        std::cout<<send_str<<std::endl;
    }
    //反序列化
    GetFriednListsResponse rspB;
    if(rspB.ParseFromString(send_str))
    {
        std::cout<<rspB.friend_list(0).name()<<std::endl;
        std::cout<<rspB.friend_list(1).age()<<std::endl;
    }
    return 0;
}