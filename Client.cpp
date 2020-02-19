#include "Client.h"

Client::Client() //写是写出来了 我在想这个构造函数有没有意义
    {
        sock_fd = -1;
        epoll_fd = -1;
        pipe_fd[2] = {0};
        memset(&acc, 0, sizeof(acc));
        isLogin = false;
    }
    void Client::Connect() //连接服务器用函数
    {
        clear();
        char ip[17] = {0};
        int port;
        cout << "请输入服务器IP：" << endl;
        cin.clear();
        cin.sync();
        cin >> ip;
        getchar();
        cout << "请输入端口号：" << endl;
        cin.clear();
        cin.sync();
        cin >> port;
        getchar();
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
        serv_addr.sin_addr.s_addr = inet_addr(ip);
        serv_addr.sin_port = htons((unsigned)port);
        for (int i = 0; i <= 3; i++)
        {
            cout << "Connecting……" << endl;
            if (connect(sock_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
            {
                cout << "Connecting error;" << endl;
                if (i == 3)
                {
                    cout << "Please try to restart client!" << endl;
                    user_wait();
                    exit(-1);
                }
                else
                {
                    cout << "Try to reconnect;" << endl;
                }
            }
            else
            {
                break;
            }
        }
        if (pipe(pipe_fd) < 0) //多进程时使用（建立管道描述符）
        {
            cout << "Other error;" << endl;
            user_wait();
            exit(-1);
        }
        epoll_fd = epoll_create(1024);
        if (epoll_fd < 0)
        {
            cout << "Other error;" << endl;
            user_wait();
            exit(-1);
        }
        addepollfd(epoll_fd, sock_fd);
        addepollfd(epoll_fd, pipe_fd[0]);
        cout << "Connected." << endl;
        user_wait();
    }
    void Client::Login() //表登录接口
    {
        clear();
        int i;
        cout << "是否拥有聊天室账户？" << endl;
        cout << "1.我已拥有账户，希望登录。" << endl;
        cout << "2.我还没有聊天室账户，希望注册。" << endl;
        while (1)
        {
            cin.clear();
            cin.sync();
            cin >> i;
            getchar();
            if (i == 1 || i == 2)
            {
                switch (i)
                {
                case 1:
                    clear();
                    char password[14];
                    cout << "请输入用户名：" << endl;
                    cin.clear();
                    cin.sync();
                    cin >> this->acc.account;
                    getchar();
                    strcpy(password, getpass("请输入密码："));
                    strcpy(this->acc.pwd, crypt(password, password));
                    acc.flag = true;
                    Login(this->acc);
                    break;
                case 2:
                    Signup();
                    break;
                }
                break;
            }
            else
            {
                cout << "输入错误，请重新输入！" << endl;
            }
        }
    }
    void Client::Login(Account acc) //重载里登录接口
    {
        char buf[65535];
        clear();
        cout << "登录中……" << endl;
        memset(&msg, 0, sizeof(msg));
        msg.type = COMMAND;
        strcpy(msg.content, "SIGNIN");
        if (sendMsg(msg, sock_fd) > 0) //发送登入请求
        {
            memset(buf, 0, sizeof(buf));
            memcpy(buf, &acc, sizeof(acc));
            send(sock_fd, buf, sizeof(buf), 0);
            recvMsg(sock_fd, msg); //这句有bug
            if (strstr(msg.content, "SUCCESS") != NULL)
            {
                int i;
                isLogin = true;
                cout << "登录成功！" << endl;
                cout << "是否在本地保存登陆凭据？" << endl;
                cout << "1.是" << endl;
                cout << "2.否" << endl;
                cin.clear();
                cin.sync();
                cin >> i;
                getchar();
                switch (i)
                {
                case 1:
                {
                    fstream userinfo;
                    userinfo.open("userinfo", ios::out);
                    userinfo << "ACC:";
                    userinfo << acc.account << endl;
                    userinfo << "PWD:";
                    userinfo << acc.pwd;
                    userinfo.close();
                    break;
                }
                case 2:
                    break;
                }
            }
            else
            {
                cout << msg.content << endl;
                cout << "登录失败，请检查账号或密码是否正确！" << endl;
                user_wait();
                Login();
            }
        }
        clear();
    }
    void Client::Signup()
    {
        clear();
        char nickname[20];
        char password1[32];
        char password2[32];
        struct Account tmp;
        cout << "请输入昵称：" << endl;
        cin.clear();
        cin.sync();
        cin >> nickname;
        getchar();
        while (1)
        {
            strcpy(password1, getpass("请输入密码："));
            strcpy(password2, getpass("请再次输入密码："));
            if (strcmp(password1, password2) == 0)
            {
                strcpy(tmp.account, nickname);
                strcpy(tmp.pwd, crypt(password1, password1));
                tmp.flag = true;
                char buf[65535];
                cout << "注册中……" << endl;
                memset(&msg, 0, sizeof(msg));
                msg.type = COMMAND;
                strcpy(msg.content, "SIGNUP");
                if (sendMsg(msg, sock_fd) > 0) //发送注册请求
                {
                    memset(buf, 0, sizeof(buf));
                    memcpy(buf, &tmp, sizeof(tmp));
                    send(sock_fd, buf, sizeof(buf), 0);
                    recvMsg(sock_fd, msg);
                    if (strstr(msg.content, "SUCCESS") != NULL)
                    {
                        memset(buf, 0, sizeof(buf));
                        memset(&acc, 0, sizeof(acc));
                        strcpy(acc.account, tmp.account);
                        strcpy(acc.pwd, tmp.pwd);
                        cout << "注册成功！" << endl;
                        cout << "您的用户名是：" << acc.account << endl;
                        cout << "您的用户名是您登入聊天室的唯一凭据，请妥善保管您的用户名与密码！" << endl;
                        user_wait();
                        acc.flag = true;
                        Login(acc);
                    }
                    else
                    {
                        cout << msg.content << endl;
                        cout << "请求提交失败，请稍后重试……" << endl;
                        user_wait();
                    }
                }
                break;
            }
            else
            {
                cout << "两次密码输入不一致，请重新输入……" << endl;
                user_wait();
            }
        }
    }
    void Client::fileLogin()
    {
        while (isLogin == false)
        {
            fstream userinfo;
            userinfo.open("userinfo", ios::in);
            if (!userinfo)
            {
                userinfo.close();
                Login();
            }
            else
            {
                userinfo.close();
                while (1)
                {
                    clear();
                    cout << "本地已有账户登录信息，是否登录？" << endl;
                    cout << "1.是" << endl;
                    cout << "2.否" << endl;
                    int i;
                    cin.clear();
                    cin.sync();
                    cin >> i;
                    getchar();
                    if (i == 1 || i == 2)
                    {
                        switch (i)
                        {
                        case 1:
                        {
                            string tmp;
                            fstream userinfo;
                            userinfo.open("userinfo", ios::in);
                            getline(userinfo, tmp);
                            tmp.erase(0, 4);
                            strcpy(acc.account, tmp.c_str());
                            getline(userinfo, tmp);
                            tmp.erase(0, 4);
                            strcpy(acc.pwd, tmp.c_str());
                            acc.flag = true;
                            Login(acc);
                            break;
                        }
                        case 2:
                            Login();
                            break;
                        }
                        break;
                    }
                    else
                    {
                        cout << "输入非法，请重新输入！" << endl;
                    }
                }
            }
        }
    }
    void Client::Start()
    {
        static struct epoll_event events[2];
        char buf[65535];
        //Getfriendlist();
        Connect();
        if (LOGINMODE)
        {
            fileLogin();
        }
        else
        {
            string tmp;
            cout << "请输入昵称：" << endl;
            getline(cin, tmp, '\n'); //非登录模式下获取用户名
            strcpy(acc.account, tmp.c_str());
            isLogin = true;
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
            cout << "全服广播(请直接在下方输入消息，enter键发送)>" << endl;
            while (isLogin)
            {
                string tmp;
                getline(cin, tmp, '\n'); //按行读取消息 不跳空格的那种
                /*广播消息部分开始*/
                memset(&msg, 0, sizeof(msg));
                msg.type = ALL;
                strcpy(msg.fromUser, acc.account);
                strcpy(msg.content, tmp.c_str());
                memset(buf, 0, 65535);
                memcpy(buf, &msg, sizeof(msg));
                write(pipe_fd[1], buf, sizeof(buf));
                /*广播消息部分结束*/
            }
        }
        else //父进程执行区（pid > 0）
        {
            close(pipe_fd[1]); //关闭写端
            while (isLogin)
            {
                int epoll_events_count = epoll_wait(epoll_fd, events, 2, -1);
                for (int i = 0; i < epoll_events_count; i++)
                {
                    memset(buf, 0, sizeof(buf));
                    if (events[i].data.fd == sock_fd) //epoll响应来自服务器标识符时
                    {
                        recvMsg(sock_fd, msg);
                        cout << ">" << msg.fromUser << " 说：" << msg.content << endl;
                    }
                    else //管道标识符响应时
                    {
                        read(events[i].data.fd, buf, 65535);
                        if (send(sock_fd, buf, sizeof(buf), 0) < 0)
                        {
                            cout << "发送失败！" << endl;
                            cout << "服务器连接失败，请检查网络连接！" << endl;
                        }
                    }
                }
            }
        }
    }
    /*===========================================待完成部分===========================================*/
    /*好友列表部分*/
    void Client::makeFriend() //添加好友
    {
    }
    void Client::deleteFriend() //删除好友
    {
    }
    void Client::queryFriendList() //请求好友列表
    {
    }
    void Client::setSuki() //设为特别关心
    {
    }
    void Client::setKirai() //拉黑名单
    {
    }
    /*好友列表部分*/

    /*群聊相关权限部分*/
    void Client::createGroupTalk() //创建群聊
    {
    }
    void Client::setGroupAdmin() //设置管理员
    {
    }
    void Client::joinGroup() //加入群聊
    {
    }
    void Client::leaveGroup() //主动离开群聊
    {
    }
    void Client::deleteGroupMember() //踢人
    {
    }
    void Client::Grouptalk() //处理用户群聊请求
    {
    }
    /*群聊与相关权限部分*/

    /*私聊部分*/
    void Client::Privatetalk() //处理用户私聊请求
    {
    }
    /*私聊部分*/
    /*===========================================待完成部分===========================================*/
    Client::~Client()
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