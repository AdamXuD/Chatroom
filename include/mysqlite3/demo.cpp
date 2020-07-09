#include <iostream>
#include "mysqlite3.h"

using namespace std;
/*这是一个使用小demo*/
int main(int, char **)
{
    Mysqlite3 db("test.db");
    db.query("CREATE TABLE `userinfo` ( `account` varchar(32) NOT NULL DEFAULT '', `pwd` varchar(32) DEFAULT NULL, PRIMARY KEY (`account`) );");
    db.query("INSERT INTO userinfo values ('123', '456');");
    db.query("INSERT INTO userinfo values ('789', '101');");
    db.query("SELECT * FROM userinfo;");

    for(int i = 0; i < (*db.fetch_data()).size(); i++)
    {
        for(int j = 0; j < (*db.fetch_data())[i].size(); j++)
        {
            cout << (*db.fetch_data())[i][j] << " ";
        }
        cout << endl;
    }

    cout << db.error() << endl;
    return 0;
}
/*这是一个使用小demo*/

