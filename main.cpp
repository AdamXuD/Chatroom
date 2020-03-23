#include "Client.h"
#include "Server.h"

int main()
{
    int choice;
    cout << "请选择运行模式：" << endl;
    cout << "1.客户端" << endl;
    cout << "2.服务端" << endl;
    input(choice);
    if (choice == 1 || choice == 2)
    {
        switch (choice)
        {
            case 1: {Client client; client.Start(); break; }
            case 2: {Server server; server.Start(); break; }
        }
    }
    return 0;
}
