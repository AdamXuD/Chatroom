#pragma once

#include <iostream>
#include <vector>
#include <string>
#include "sqlite3.h"

// 这是一个由AdamXuD实现的一个工具类
// 旨在简化sqlite3的使用
// 以及练习lambda表达式和回调函数
// 还有基于vector的动态二维数组
// 使其语法更接近与mysql
// 虽然说返回一个指针数据感觉更复杂了（x
// 需要链接pthread和dl才能使用（sqlite3库的要求


class Mysqlite3
{
public:
    Mysqlite3();                  //构造
    Mysqlite3(const char *_file); //构造
    ~Mysqlite3();                 //析构

    int open(const char *_file);   //打开数据库文件，没有则创建
    char *error();                 //返回上一条错误语句的错误援引
    int query(const char *_query); //执行sql语句

    std::vector<std::vector<std::string>> *fetch_data(); //返回一个包含有

private:
    sqlite3 *db;
    char *err;

    std::vector<std::vector<std::string>> *data;

protected:
};