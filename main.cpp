#include "Client.h"
#include "Server.h"
#include "Common.h"

int main(int argc, char *argv[])
{
    if (argc == 2)
    {
        if (strEqual(argv[1], "--init"))
            serverInitialization();
        else
            cout << "Do you mean ./Chatroom --init?" << endl;
        return 0;
    }

    string menulist[3] = {"客户端", "服务端", "退出"};
    switch (menu(menulist, 3, "请选择运行模式："))
    {
    case 1:
    {
        Client client;
        client.Start();
        break;
    }
    case 2:
    {
        Server server;
        server.Start();
        break;
    }
    default:
    {
        return 0;
    }
    }
    return 0;
}