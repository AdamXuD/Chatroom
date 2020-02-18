#include<iostream>
#include<fstream>
#include<sys/socket.h>
#include<string>
#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<stdlib.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<sys/epoll.h>
#include<crypt.h>
#include<fcntl.h>
#include<mysql/mysql.h>
#include<map>  //提供一一对应的存储容器
//#include<ncurses.h>  //用户界面库 到时候有余裕的话咱可以整整
#define LOGINMODE 0  //默认关闭登录模式（因为电脑上不一定有这个服务器启动所要求的数据库）

/*消息类型宏定义开始（防止自己忘记哪个数字对应哪个消息类型）*/
#define COMMAND 1
#define ALL 2
#define PRIVTALK 3
#define GROUPTALK 4
/*消息类型宏定义结束*/
using namespace std;
/*公用消息结构定义开始*/
//信息类
struct Msg //信息类所有信息都用这个结构发送
{
    int type;  //消息类型
    char toUser[32]; //目标账号
    char fromUser[32];  //用户来源
    char content[4096];  //内容
};
//账户类(登陆注册时向服务器传这个结构体)
struct Account
{
    bool flag; //用这个来判断传输的账户类是否为空(感觉有点多余)
    char account[32];
    char pwd[32];
};
/*公用消息结构定义结束*/

/*双端通用函数开始*/
//清屏函数
void clear()//证明我曾经希望过它能跨平台使用
{
#ifdef __GNUC__
    system("clear");
#elif defined _MSC_VER
    system("cls");
#endif
}
//等待用户阅读结果
void user_wait()//等待接口
{
    cout << "请按任意键继续……" << endl;
    getchar();
    clear();
}
/*===========================================待完成部分===========================================*/
//发送信息
int sendMsg(const Msg msg, int fd) //发送消息用接口
{

}
//接收信息
int recvMsg(int fd, Msg& msg)  //接收消息用接口
{

}
/*===========================================待完成部分===========================================*/
//增加监听socket的数量
//写完这个函数我就知道跨平台有点难了 听说Windows有iocp？？？
static void addepollfd(int epoll_fd, int fd)
{
    struct epoll_event tmp;  //设置临时变量用于存储描述符属性
    tmp.events = EPOLLIN | EPOLLET;  //设置epoll模式为et
    tmp.data.fd = fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &tmp);
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0) | O_NONBLOCK); //读取标识符状态并通过位运算设置其为无阻塞
}
/*双端通用函数结束*/

/*客户端类开始*/
class Client
{
public:
    Client() //写是写出来了 我在想这个构造函数有没有意义
    {
        sock_fd = -1;
        epoll_fd = -1;
        pipe_fd[2] = { 0 };
        memset(&acc, 0, sizeof(acc));
        isLogin = false;
    }
    void Connect() //连接服务器用函数
    {
        clear();
        char ip[17] = { 0 };
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
            if (connect(sock_fd, (struct sockaddr*) & serv_addr, sizeof(serv_addr)) < 0)
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
        if (pipe(pipe_fd) < 0)//多进程时使用（建立管道描述符）
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
    void Login() //表登录接口
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
    void Login(Account acc) //重载里登录接口
    {
        char buf[65535];
        clear();
        cout << "登录中……" << endl;
        memset(&msg, 0, sizeof(msg));
        msg.type = COMMAND;
        strcpy(msg.content, "SIGNIN");
        if (sendMsg(msg, sock_fd) > 0)  //发送登入请求
        {
            memset(buf, 0, sizeof(buf));
            memcpy(buf, &acc, sizeof(acc));
            send(sock_fd, buf, sizeof(buf), 0);
            recvMsg(sock_fd, msg);                   //这句有bug
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
    void Signup()
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
                if (sendMsg(msg, sock_fd) > 0)  //发送注册请求
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
    void fileLogin()
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
    void Start() {
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
            getline(cin, tmp, '\n');//非登录模式下获取用户名
            strcpy(acc.account, tmp.c_str());
            isLogin = true;
        }
        pid = fork();
        if (pid < 0)
        {
            cout << "程序出错，程序即将退出！" << endl;
            exit(-1);
        }
        else if (pid == 0)  //子进程执行区（pid = 0）
        {
            close(pipe_fd[0]);  //子进程负责写入，关闭读端
            cout << "全服广播(请直接在下方输入消息，enter键发送)>" << endl;
            while (isLogin)
            {
                string tmp;
                getline(cin, tmp, '\n');//按行读取消息 不跳空格的那种
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
        else//父进程执行区（pid > 0）
        {
            close(pipe_fd[1]);  //关闭写端
            while (isLogin)
            {
                int epoll_events_count = epoll_wait(epoll_fd, events, 2, -1);
                for (int i = 0; i < epoll_events_count; i++)
                {
                    memset(buf, 0, sizeof(buf));
                    if (events[i].data.fd == sock_fd)  //epoll响应来自服务器标识符时
                    {
                        recvMsg(sock_fd, msg);
                        cout << ">" << msg.fromUser << " 说：" << msg.content << endl;
                    }
                    else  //管道标识符响应时
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
    void makeFriend() //添加好友
    {

    }
    void deleteFriend() //删除好友
    {

    }
    void queryOnlineMember()
    {

    }
    void setSuki() //设为特别关心
    {

    }
    void setKirai()  //拉黑名单
    {

    }
    /*好友列表部分*/
    /*群聊相关权限部分*/
    void createGroupTalk()  //创建群聊
    {

    }
    void setGroupAdmin() //设置管理员
    {

    }
    void joinGroup() //加入群聊
    {

    }
    void leaveGroup()  //主动离开群聊
    {

    }
    void deleteGroupMember()  //踢人
    {

    }
    void Grouptalk()//处理用户群聊请求
    {

    }
    /*群聊与相关权限部分*/
    /*私聊部分*/
    void Privatetalk()//处理用户私聊请求
    {

    }
    /*私聊部分*/
    /*===========================================待完成部分===========================================*/
    ~Client()
    {
        if (pid > 0)
        {
            close(pipe_fd[0]); //父进程关闭管道读端
            close(sock_fd);  //关闭socket句柄
        }
        else
        {
            close(pipe_fd[1]);  //子进程关闭写端
        }
    }
private:
    int pipe_fd[2]; //读写管道pipe()……
    int pid;//多进程时fork()返回进程号时用得到
    int sock_fd; //要连接的服务器句柄（句柄应该是这个意思吧
    int epoll_fd;//epoll二叉树根的句柄，真的玄妙
    bool isLogin;//登陆状态
    map<int, string> onlinelist; //用map记录在线列表（包含名称和）（一一对应）
    struct Msg msg;//消息
    struct Account acc;//账号信息
protected:
};
/*客户端类结束*/
/*服务端类开始*/
class Server
{
public:
    Server()
    {
        sock_fd = 0;
        epoll_fd = 0;
    }
    void Prepare()
    {
        clear();
        char ip[17] = { 0 };
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
        cout << "服务器启动中……" << endl;
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
        if (bind(sock_fd, (struct sockaddr*) & serv_addr, sizeof(serv_addr)) < 0)
        {
            cout << "Socket errer;" << endl;
            user_wait();
            exit(-1);
        }
        if (listen(sock_fd, 5) < 0)
        {
            cout << "Change to LISTENING errer;" << endl;
            user_wait();
            exit(-1);
        }
        else
        {
            cout << "Server is LISTENING..." << endl;
        }
        epoll_fd = epoll_create(4096);
        if (epoll_fd < 0)
        {
            cout << "Other error" << endl;
            user_wait();
            exit(-1);
        }
        addepollfd(epoll_fd, sock_fd);
        /*数据库初始化部分开始*/
        if (LOGINMODE)
        {
            mysql_init(&mysql);
            if (!mysql_real_connect(&mysql, "127.0.0.1", "root", "root", "Chatroom", 3306, NULL, 0))
            {
                cout << "Failed to connect to database: Error:" << mysql_error(&mysql) << endl;
                user_wait();
                exit(-1);
            }
            else
            {
                cout << "Success to connect database" << endl;
            }
        }
        /*数据库初始化部分结束*/
        user_wait();
    }
    void addonlinelist(int clnt_fd, char* acc)
    {
        if (onlinelist.count(clnt_fd) == 0)
        {
            onlinelist[clnt_fd] = acc;
        }
        else
        {
            onlinelist.erase(clnt_fd);
            onlinelist[clnt_fd] = acc;
        }
    }
    void BroadcastMsg(int call)
    {
        cout << "A user sends a broadcast message." << endl;
        map<int, string>::iterator i;  //声明一个迭代器（相当于int i
        for (i = onlinelist.begin(); i != onlinelist.end(); i++)
        {
            if (i->first != call)//除了发信者外
            {
                cout << "Send message to client " << i->first << endl;
                sendMsg(msg, i->first);//给所有在线玩家发送消息
            }
        }
    }
    void Onlineremind(int call)
    {
        memset(&msg, 0, sizeof(msg));
        msg.type = ALL;
        strcpy(msg.fromUser, "Admin");
        strcpy(msg.content, "Someone is Online.");
        BroadcastMsg(call);
    }
    void Login(int call)  //登录处理函数
    {
        cout << "A connector is trying to login." << endl;
        char buf[65535];
        char query[1024];
        memset(buf, 0, sizeof(buf));
        memset(&acc, 0, sizeof(acc));
        while (acc.flag == 0)
        {
            recv(call, buf, 65535, 0);
            memcpy(&acc, buf, sizeof(acc));
        }
        sprintf(query, "select account, pwd from userinfo where account='%s';", acc.account);
        if (mysql_query(&mysql, query) == 0)
        {
            MYSQL_RES res;
            MYSQL_ROW row;
            res = *mysql_store_result(&mysql);
            row = mysql_fetch_row(&res);
            if (row != NULL)
            {
                if (strstr(acc.pwd, row[1]) != NULL)
                {
                    memset(&msg, 0, sizeof(msg));
                    msg.type = COMMAND;
                    strcpy(msg.content, "SUCCESS");
                    sendMsg(msg, call); //服务端反馈
                    cout << "Login success." << endl;
                    onlinelist[call] = acc.account;
                    cout << "Now there are " << onlinelist.size() << " user(s) online." << endl;
                    Onlineremind(call);
                }
                else
                {
                    memset(&msg, 0, sizeof(msg));
                    msg.type = COMMAND;
                    strcpy(msg.content, "FAILED:Login failed.Info does not match.");
                    sendMsg(msg, call); //服务端反馈
                    cout << "Login failed.Info does not match." << endl;
                }
            }
            else
            {
                memset(&msg, 0, sizeof(msg));
                msg.type = COMMAND;
                strcpy(msg.content, "FAILED:Login failed.Info does not match.");
                sendMsg(msg, call); //服务端反馈
                cout << "Login failed.Info does not match." << endl;
            }
        }
        else
        {
            char tmp[1024] = { 0 };
            strcpy(tmp, "FAILED:Login failed.Database error:");
            strcat(tmp, mysql_error(&mysql));
            memset(&msg, 0, sizeof(msg));
            msg.type = COMMAND;
            strcpy(msg.content, tmp);
            sendMsg(msg, call); //服务端反馈
            cout << tmp << endl;
        }
    }
    void Signup(int call)
    {
        cout << "A connector is trying to signup." << endl;
        char buf[65535];
        char query[1024];
        memset(buf, 0, sizeof(buf));
        memset(&acc, 0, sizeof(acc));
        while (acc.flag == false)
        {
            usleep(1000);
            recv(call, buf, 65535, 0);
            memcpy(&acc, buf, sizeof(acc));
        }
        sprintf(query, "insert into userinfo values ('%s', '%s');", acc.account, acc.pwd);
        if (mysql_query(&mysql, query) == 0)  //将用户名密码等数据写入数据库
        {
            memset(&msg, 0, sizeof(msg));
            msg.type = COMMAND;
            strcpy(msg.content, "SUCCESS");
            sendMsg(msg, call); //服务端反馈
            cout << "Signup success." << endl;
        }
        else
        {
            memset(&msg, 0, sizeof(msg));
            msg.type = COMMAND;
            strcpy(msg.content, "FAILED");
            strcat(msg.content, mysql_error(&mysql));
            sendMsg(msg, call); //服务端反馈
            cout << "Signup failed:" << mysql_error(&mysql) << endl;
        }

    }
    /*===========================================待完成部分===========================================*/
    /*好友列表部分*/
    void makeFriend() //添加好友
    {

    }
    void deleteFriend() //删除好友
    {

    }
    void setSuki() //设为特别关心
    {

    }
    void setKirai()  //拉黑名单
    {

    }
    void createFriendList()//注册完成时创建该用户的好友列表
    {

    }
    void sendFriendList() //发送好友列表（登录后马上调用，以获取该用户的好友列表）
    {

    }
    /*好友列表部分*/
    /*群聊及其权限部分*/
    void createGroupTalk()  //创建群聊
    {

    }
    void setGroupAdmin() //设置管理员
    {

    }
    void joinGroup()//处理加入群聊请求
    {

    }
    void leaveGroup()//处理离开群聊请求
    {

    }
    void deleteGroupMember()//处理踢出群聊请求
    {

    }
    void Grouptalk()//处理用户群聊请求
    {

    }
    /*群聊及其权限部分*/
    /*私聊部分*/
    void Privatetalk()//处理用户私聊请求
    {

    }
    /*私聊部分*/
    /*===========================================待完成部分===========================================*/
    void dealWithMsg(int call)
    {
        switch (msg.type)
        {
            case 0:
            {
                cout << "illegal message:" << msg.content << endl;
                break;
            }
            case COMMAND:   //如果收到命令类消息
            {
                if (strstr(msg.content, "SIGNIN") != NULL)//接收并处理客户端的登录请求
                {
                    Login(call);
                }
                else if (strstr(msg.content, "SIGNUP") != NULL)//接收并处理客户端的注册请求
                {
                    Signup(call);
                }
                else  //还可以再添加其他的else 我想通过这种方式来实现组群加好友之类的操作 但是我觉得这样子效率有点低 你们看看有没有更好的方法
                {
                    cout << "illegal message:" << msg.content << endl;
                }
                break;
            }
            case ALL:
            {
                cout << "From:" << msg.fromUser << endl;
                cout << "Say:" << msg.content << endl;
                BroadcastMsg(call);
                break;
            }
            case PRIVTALK:  //下面两个可能写不完 先写
            {
                cout << "A user sends a message to another user." << endl;
                Privatetalk();
                break;
            }
            case GROUPTALK:
            {
                cout << "A user sends a message to a group" << endl;
                Grouptalk();
                break;
            }
        }
    }
    void Start()//服务端程序入口
    {
        static struct epoll_event events[16384];  //设置标识符监听队列
        Prepare();
        while (1)
        {
            int events_counter = epoll_wait(epoll_fd, events, 16384, -1); //在接收到事件之前保持阻塞
            for (int i = 0; i < events_counter; i++)
            {
                int call = events[i].data.fd;
                if (call == sock_fd) //如果事件是由服务器bind的socket上发生的
                {
                    struct sockaddr_in clnt_addr;
                    memset(&clnt_addr, 0, sizeof(clnt_addr));
                    socklen_t addr_len = sizeof(struct sockaddr_in);
                    int clnt_sock = accept(sock_fd, (struct sockaddr*) & clnt_addr, &addr_len); //接收第一次连接的客户端并在服务端上为其分配新的socket（离谱为什么bind只需要地址长度的值而accept要引用）
                    cout << "Get a new Connect from:" << inet_ntoa(clnt_addr.sin_addr) << ":" << ntohs(clnt_addr.sin_port) << "..." << endl;
                    cout << "Waiting for login……" << endl;
                    addepollfd(epoll_fd, clnt_sock);  //将分配后的标识符加入监听队列
                    if (LOGINMODE == 0)
                    {
                        cout << "Login success." << endl;
                        onlinelist[clnt_sock] = acc.account;
                        cout << "Now there are " << onlinelist.size() << " user(s) online." << endl;
                        Onlineremind(clnt_sock);
                    }
                }
                else  //如果服务器来自epoll监控队列中的其他描述符（已被accept分配过新的标识符）
                {
                    if (recvMsg(call, msg) < 0)
                    {
                        cout << "Receive error." << endl;
                    }
                    else
                    {
                        dealWithMsg(call);
                    }
                }
            }
        }
    }
    ~Server()
    {
        close(sock_fd);
        close(epoll_fd);
    }
private:
    int sock_fd; //socket产生的句柄
    int epoll_fd; //epoll_create()返回的句柄
    map<int, string> onlinelist; //用map记录当前在线用户和socket（一一对应）
    Msg msg; //消息
    Account acc; //账号信息
    MYSQL mysql; //mysql句柄
protected:
};
/*服务端类结束*/
int main()
{
    int choice;
    cout << "请选择运行模式：" << endl;
    cout << "1.客户端" << endl;
    cout << "2.服务端" << endl;
    cin.clear();
    cin.sync();
    cin >> choice;
    getchar();
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
