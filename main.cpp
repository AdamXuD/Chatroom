#include "Client.h"
#include "Server.h"

int main()
{
    string menulist[3] = {"客户端", "服务端", "退出"};
    switch (menu(menulist, 3, "请选择运行模式："))
    {
        case 1: { Client client; client.Start(); break; }
        case 2: { Server server; server.Start(); break; }
        default:{ return 0; }
    }
    return 0;
}
