#include "test.pb.h"
#include <iostream>
#include <string>

int main() {
    /*fixbug::LoginRequest req;
    req.set_name("zhang san");
    req.set_pwd("123456");
    std::string send_str;
    if (req.SerializeToString(&send_str)) {
        std::cout << send_str.c_str() << std::endl;
    }

    fixbug::LoginRequest reqB;
    if (reqB.ParseFromString(send_str)) {
        std::cout << "name: " << reqB.name() << std::endl << "pwd: " << reqB.pwd() << std::endl;
    }*/

    fixbug::LoginResponse rsp;
    fixbug::ResultCode * rc = rsp.mutable_result();
    rc->set_errcode(1);
    rc->set_errmsg("登录失败");


    fixbug::GetFriendListsResponse rspB;
    fixbug::ResultCode * rcB = rspB.mutable_result();
    rcB->set_errcode(0);

    fixbug::User * user1 = rspB.add_friendlist();
    user1->set_name("zhang san");
    user1->set_age(20);
    user1->set_sex(fixbug::User::MAN);
    fixbug::User * user2 = rspB.add_friendlist();
    user2->set_name("zhang san");
    user2->set_age(20);
    user2->set_sex(fixbug::User::MAN);

    std::cout << rspB.friendlist_size() << std::endl;

    return 0;
}