#include "Client.h"

Client::Client() //构造
{
    sock_fd = -1;
    epoll_fd = -1;
    pipe_fd[2] = {0};
    memset(&acc, 0, sizeof(acc));
    isLogin = false;
    Querybox = new map<int, string>;
    friendlist = new map<string, int>;
}

void Client::Connect() //连接服务器用函数
{
    clear();
    // char ip[17] = {0};
    // int port;
    // input(ip, "请输入服务器IP：");
    // input(port, "请输入端口号：");

    Json::Reader reader;
    Json::Value root;
    ifstream config("config.json");
    if (!config)
    {
        char ip[17] = {0};
        int port;
        input(ip, "请输入服务器IP：");
        input(port, "请输入端口号：");
        newJson(ip, port);
        config.open("config.json");
    }
    reader.parse(config, root);

    sock_fd = socket(AF_INET, SOCK_STREAM, 0); //socket部分开始
    if (sock_fd < 0)
    {
        cout << "Socket error;" << endl;
        user_wait();
        exit(-1);
    }

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(root["Client"]["ServerIP"].asCString());
    serv_addr.sin_port = htons(root["Client"]["Port"].asUInt());

    for (int i = 0; i <= 3; i++)
    {
        cout << "连接中……" << endl;
        if (connect(sock_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        {
            cout << "连接失败，" << endl;
            if (i == 3)
            {
                cout << "请尝试重新启动客户端，或检查配置文件是否正确填写!" << endl;
                user_wait();
                exit(-1);
            }
            else
            {
                cout << "正在尝试第 " << i - 1 << "次重连……" << endl;
            }
        }
        else
        {
            break;
        }
    }

    if (pipe(listpipe_fd) < 0)
    {
        cout << "出现其他错误，程序中止！" << endl;
        user_wait();
        exit(-1);
    }
    if (pipe(pipe_fd) < 0) //多进程时使用（建立管道描述符）
    {
        cout << "出现其他错误，程序中止！" << endl;
        user_wait();
        exit(-1);
    }
    epoll_fd = epoll_create(1024);
    if (epoll_fd < 0)
    {
        cout << "出现其他错误，程序中止！" << endl;
        user_wait();
        exit(-1);
    }
    addepollfd(epoll_fd, sock_fd);
    addepollfd(epoll_fd, pipe_fd[0]);

    cout << "已连接上服务器" << root["Client"]["ServerIP"].asCString() << ":" << root["Client"]["Port"].asUInt() << endl;
    config.close();
    user_wait();
}
void *HeartBeat(void *pointer)
{
    int count = 0;
    Client *ptr = (Client *)pointer;
    while (count < 3)
    {
        if (sendHeartBeats(ptr->sock_fd, ptr->msg) < 0)
        {
            cout << "与服务器断开连接，正在尝试第 " << count + 1 << " 次重连..." << endl;
            count++;
        }
        sleep(3);
    }
    if (count >= 3)
    {
        cout << "重连失败，程序即将退出！" << endl;
        exit(-1);
    }
    return nullptr;
}

void Client::dealWithQuery()
{
    getQueryBox(false);
    map<int, string>::iterator it;
    for (it = Querybox->begin(); it != Querybox->end(); it++)
    {
        cout << "id = " << it->first << endl;
        cout << "内容：" << it->second << endl;
        cout << endl;
    }
    if (Querybox->size() == 0)
    {
        cout << "请求列表为空！" << endl;
        user_wait();
    }
    else
    {
        char choice[10];
        do
        {
            input(choice, "请输入事件id>");
        } while (Querybox->count(atoi(choice)) == 0);
        cout << "您已选择id:" << choice << "，请输入（y/n）以同意/拒绝该事件。" << endl;
        while (char c = getch())
        {
            if (c == 'y' || c == 'Y')
            {
                cout << "您已同意该事件。" << endl;
                setMsg(msg, ACCEPT, acc.account, nullptr, choice);
                sendMsg(msg, pipe_fd[1]);
                break;
            }
            else if (c == 'n' || c == 'N')
            {
                cout << "您已拒绝该事件。" << endl;
                setMsg(msg, REFUSE, acc.account, nullptr, choice);
                sendMsg(msg, pipe_fd[1]);
                break;
            }
            else
            {
                cout << "输入有误，请重新输入！" << endl;
            }
        }
    }
}

void Client::mainMenu()
{
    string command;
    string main_menu[] = {"发起私聊",             // 1
                          "发起群聊",             // 2
                          "添加好友",             // 3
                          "删除好友",             // 4
                          "获取请求",             // 5
                          "处理请求",             // 6
                          "设置好友为特别关心",   //7
                          "设置好友为黑名单好友", //8
                          "请求好友列表",         //9
                          "获取在线好友列表",     //10
                          "创建群组",             //11
                          "加入群组",             //12
                          "返回"};                //13
    switch (menu(main_menu, 13))                  //menu()的用法请查看注释
    {
    case 1:
    {
        command = friendlistMenu(true);
        if (strEqual(command, "__nullstr") == false)
        {
            Privatetalk(command);
        }
        break;
    }
    case 2:
    {
        command = friendlistMenu(false);
        if (strEqual(command, "__nullstr") == false)
        {
            Grouptalk(command);
        }
        break;
    }
    case 3:
    {
        makeFriend();
        break;
    }
    case 4:
    {
        deleteFriend();
        break;
    }
    case 5:
    {
        getQueryBox(true); //获取请求列表
        break;
    }
    case 6:
    {
        dealWithQuery(); //处理请求列表
        break;
    }
    case 7:
    {
        setSuki();
        break;
    }
    case 8:
    {
        setKirai();
        break;
    }
    case 9:
    {
        queryFriendList(true);
        break;
    }
    case 10:
    {
        getOnlineFriends();
        break;
    }
    case 11:
    {
        createGroupTalk();
        break;
    }
    case 12:
    {
        joinGroup();
        break;
    }
    default:
    {
        break;
    }
    }
    cout << "全服广播(请直接在下方输入消息，enter键发送，/menu唤出菜单)>" << endl;
}

void Client::dealwithmsg(char *Target)
{
    switch (msg.type)
    {
    case ALL:
    {
        cout << "\033[31m[广播]\033[0m";
        break;
    }
    case PRIVTALK:
    {
        cout << "\033[32m[私聊]\033[0m";
        break;
    }
    case GROUPTALK:
    {
        cout << "\033[34m[群聊]\033[0m";
        break;
    }
    case QUERY:
    {
        cout << "\033[35m[请求]\033[0m";
        break;
    }
    case EOF:
    case LIST:
    {
        sendMsg(msg, listpipe_fd[1]);
        return;
    }
    case FORCE_EXIT:
    {
        cout << msg.content << endl;
        exit(0);
    }
    default:
    {
        cout << "unknown massage:" << endl;
        cout << "Type:" << msg.type << endl;
        cout << "From:" << msg.fromUser << endl;
        cout << "To:" << msg.toUser << endl;
        cout << "Content:" << msg.content << endl;
    }
    }
    cout << msg.fromUser << " 说：" << msg.content << endl;
}

void Client::Start() //客户端入口
{
    char buf[65535];
    static struct epoll_event events[2];
    Connect();
    if (LOGINMODE)
    {
        fileLogin();
    }
    else
    {
        input(acc.account, "请输入昵称："); //非登录模式下获取用户名
        isLogin = true;
    }
    pthread_t heartbeat;                                               //建立新进程
    if (pthread_create(&heartbeat, NULL, HeartBeat, (void *)this) < 0) //创建一个子进程用来发送心跳包 并维持连接状态（传递参数为该客户端对象指针）
    {
        cout << "Heartbeat thread error..." << endl;
        user_wait();
        exit(-1);
    }
    pid = fork();
    if (pid < 0)
    {
        cout << "程序出错，程序即将退出！" << endl;
        exit(-1);
    }
    else if (pid == 0) //子进程执行区（pid = 0）
    {
        close(pipe_fd[0]); //子进程负责写入，关闭读端
        close(listpipe_fd[1]);
        queryFriendList(true);
        getQueryBox(true);
        cout << "全服广播(请直接在下方输入消息，enter键发送，/menu唤出菜单)>" << endl;
        while (isLogin)
        {
            string tmp;
            getline(cin, tmp, '\n'); //按行读取指令 不跳空格的那种
            /*判断指令类型*/
            if (strEqual(tmp, "/menu")) //指令语意处理
            {
                tmp.clear();
                mainMenu();
            }
            else
            {
                setMsg(msg, ALL, acc.account, nullptr, tmp.c_str());
                sendMsg(msg, pipe_fd[1]);
            }
        }
    }
    else //父进程执行区（pid > 0）
    {
        close(pipe_fd[1]); //关闭写端
        close(listpipe_fd[0]);
        char Target[32] = {0}; //存储聊天对象
        while (isLogin)
        {
            int epoll_events_count = epoll_wait(epoll_fd, events, 10, -1);
            for (int i = 0; i < epoll_events_count; i++)
            {
                if (events[i].data.fd == sock_fd) //epoll响应来自服务器标识符时
                {
                    recvMsg(sock_fd, msg, false);
                    dealwithmsg(Target);
                }
                else //管道标识符响应时
                {
                    recvMsg(pipe_fd[0], msg, false);
                    switch (msg.type)
                    {
                    case PIPE:
                    {
                        strcpy(Target, msg.toUser);
                        break;
                    }
                    default:
                    {
                        if (sendMsg(msg, sock_fd) < 0)
                        {
                            cout << "发送失败！" << endl;
                            cout << "服务器连接失败，请检查网络连接！" << endl;
                        }
                    }
                    }
                }
            }
        }
    }
}
Client::~Client() //析构
{
    if (pid > 0)
    {
        close(pipe_fd[0]); //父进程关闭管道读端
        close(sock_fd);    //关闭socket句柄
    }
    else
    {
        close(pipe_fd[1]); //子进程关闭写端
    }
}