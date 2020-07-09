#include "mysqlite3.h"

/*@Mysqlite3    保留空白构造函数*/
Mysqlite3::Mysqlite3() {}

/*@Mysqlite3    保留空白构造函数*/
/*@_file        数据库文件名称（若本地无该数据库则新建）*/
Mysqlite3::Mysqlite3(const char *_file)
{
    sqlite3_open(_file, &db);
    err = nullptr;
    data = new std::vector<std::vector<std::string>>(0);
}

/*@~Mysqlite3    析构函数*/
Mysqlite3::~Mysqlite3()
{
    sqlite3_close(db);
    err = nullptr;
    delete data;
}


/*@open         打开数据库函数*/
/*@_file        数据库文件名称（若本地无该数据库则新建）*/
int Mysqlite3::open(const char *_file)
{
    sqlite3_open(_file, &db);
}

/*@error        返回最后一次出现的错误*/
char *Mysqlite3::error()
{
    return err;
}

/*@query        执行数据库语句*/
/*@_query       数据库语句*/
int Mysqlite3::query(const char *_query)
{
    data->clear();
    return sqlite3_exec(db, _query, [](void *data, int columeCount, char **columeValue, char **columeName) -> int {
            std::vector<std::vector<std::string>> *ptr = static_cast<std::vector<std::vector<std::string>> *>(data);

            std::vector<std::string> tmp;
            for(int i = 0; i < columeCount; i++)
            {
                tmp.push_back(columeValue[i]);
            }
            ptr->push_back(tmp);
            return 0;
        }, data, &err);
}

/*@fetch_data   返回结果集指针函数*/
std::vector<std::vector<std::string>> *Mysqlite3::fetch_data()
{
    return data;
}
