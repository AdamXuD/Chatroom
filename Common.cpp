#include "Common.h"

char content[5120];
char query[10240];

void clear()
{
#ifdef __GNUC__
    system("clear");
#elif defined _MSC_VER
    system("cls");
#endif
}

void user_wait() //等待接口
{
    cout << "请按任意键继续……" << endl;
    getchar();
    clear();
}

void input(char *ptr, const char *tips) //输入框
{
    if (tips != nullptr)
    {
        cout << tips << endl;
    }
    cin.getline(ptr, 65535);
}

void input(int &num, const char *tips)
{
    char tmp[10];
    if (tips != nullptr)
    {
        cout << tips << endl;
    }
    cin.getline(tmp, 10);
    num = atoi(tmp);
}

int sendMsg(Msg &msg, int fd) //发送消息用接口
{
    char buf[65535];
    memset(buf, 0, sizeof(buf));
    memcpy(buf, &msg, sizeof(msg));
    int ret = write(fd, buf, sizeof(buf));
    return ret;
}

int recvMsg(int fd, Msg &msg, bool wait) //接收消息用接口
{
    char buf[65535];
    int ret = 0;
    memset(&msg, 0, sizeof(msg));
    while (ret <= 0)
    //因为设定了socket非阻塞状态 read()函数未接受到消息也不再阻塞
    //设定循环让其接收到消息后再跳出循环
    {
        usleep(1000);
        //然后就是这个死循环要是不用sleep函数放慢一点的话我后面的函数一定会出问题(就很离谱)
        //而且会对资源造成不必要的浪费
        memset(buf, 0, sizeof(buf));
        ret = read(fd, buf, 65535);
        memcpy(&msg, buf, sizeof(msg));
        if (!wait)
        {
            break;
        }
    }
    return ret;
}

void setMsg(Msg &msg, int type, const char *fromUser, const char *toUser, const char *content)
{
    memset(&msg, 0, sizeof(msg));
    msg.type = type;
    if (fromUser != nullptr)
    {
        strcpy(msg.fromUser, fromUser);
    }
    if (toUser != nullptr)
    {
        strcpy(msg.toUser, toUser);
    }
    if (content != nullptr)
    {
        strcpy(msg.content, content);
    }
}

bool strEqual(string str1, const char *str2) //判断字符串是否相等（或1是否包含2）
{
    if (str1.find(str2) != string::npos)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool strEqual(const char *str1, const char *str2) //判断字符串1是否包含2
{
    if (strstr(str1, str2) != NULL)
    {
        return true;
    }
    else
    {
        return false;
    }
}

int sendHeartBeats(int fd, Msg &msg)
{
    memset(&msg, 0, sizeof(msg));
    msg.type = HEARTBEAT;
    int ret = sendMsg(msg, fd);
    memset(&msg, 0, sizeof(msg));
    return ret;
}

void addepollfd(int epoll_fd, int fd) //增加监听描述符
{
    struct epoll_event tmp;         //设置临时变量用于存储描述符属性
    tmp.events = EPOLLIN | EPOLLET; //设置epoll模式为et
    tmp.data.fd = fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &tmp);
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0) | O_NONBLOCK); //读取标识符状态并通过位运算设置其为无阻塞
}

int input()
{
    int i;
    cin >> i;
    getchar();
    return i;
}

int Mysql_query(MYSQL *mysql, const char *q)
{
    int ret;
    if (ret = mysql_query(mysql, q) != 0)
    {
        cout << "Error at Line: " << q << ".";
        cout << "Error:" << mysql_error(mysql) << endl;
    }
    return ret;
}

int getch()
{
#ifdef __GNUC__
    int cr;
    struct termios nts, ots;

    if (tcgetattr(0, &ots) < 0) // 得到当前终端(0表示标准输入)的设置
        return EOF;

    nts = ots;
    cfmakeraw(&nts);                     // 设置终端为Raw原始模式，该模式下所有的输入数据以字节为单位被处理
    if (tcsetattr(0, TCSANOW, &nts) < 0) // 设置上更改之后的设置
        return EOF;

    cr = getchar();
    if (tcsetattr(0, TCSANOW, &ots) < 0) // 设置还原成老的模式
        return EOF;

    return cr;
#elif defined _MSC_VER
    return _getch();
#endif
}

int menu(string list[], int size, char *tips) //第一个参数是菜单列表，第二个参数是菜单项数量
{
    int select = 1;
    while (1)
    {
        clear();
        if(tips != nullptr)
        {
            cout << tips << endl;
        }
        for (int i = 0; i < size; i++)
        {
            if (i == select - 1)
            {
                cout << "\033[7m"
                     << ">" << list[i] << "\033[0m" << endl;
            }
            else
            {
                cout << ">" << list[i] << endl;
            }
        }
        //上下键相当于同时按下224和（72或者80）键
        //而回车键是13
        int ch1 = getch(); //获取第一个键
        if (ch1 == 13)
        {
            return select; //如果是回车 返回菜单号
        }
        int ch2 = getch(); //如果不是回车 获取第二个键值
        int ch3 = getch();
        if (ch1 == 27 && ch2 == 91 && ch3 == 66)
        {
            select++;
        }
        else if (ch1 == 27 && ch2 == 91 && ch3 == 65) //判断按下了哪个键
        {
            select--;
        }

        if (select > size) //判断越界
        {
            select = 1;
        }
        else if (select < 1)
        {
            select = size;
        }
    }
}

void input(string &str, const char *tips) //输入框
{
    if (tips != nullptr)
    {
        cout << tips << endl;
    }
    getline(cin, str, '\n');
}

void newJson(char *IP, int port)
{
    ofstream config("config.json");
    Json::Value root;
    Json::Value server;
    Json::Value client;

    server["ListenIP"] = Json::Value(IP);
    server["Port"] = Json::Value(port);
    server["DatabaseAccount"] = Json::Value();
    server["DatabasePassword"] = Json::Value();
    server["DatabaseName"] = Json::Value();
    server["DatabasePort"] = Json::Value();
    root["Server"] = Json::Value(server);

    client["ServerIP"] = Json::Value(IP);
    client["Port"] = Json::Value(port);
    client["Account"] = Json::Value();
    client["pwd"] = Json::Value();
    root["Client"] = Json::Value(client);

    Json::StyledWriter writer;
    config << writer.write(root);
    config.close();
}

char *Getpass(const char *tips)
{
    char *ret = getpass(tips);
    while (strcmp(ret, "") == 0)
    {
        cout << "密码不允许为空！" << endl;
        user_wait();
        ret = getpass(tips);
    }
    return ret;
}

int mygetline(string &str) //支持功能键的
{
    while (char c = getch())
    {

        if (c == 27) //Esc退出实现
        {
            return EOF;
            break;
        }
        else if (c == 13) //回车实现
        {
            cout << endl;
            return str.size();
            break;
        }
        else if (c == 127) //退格实现
        {
            if (str.size() > 0)
            {
                cout << "\b";
                cout << " ";
                cout << "\b";
                str.pop_back();
            }
        }
        else
        {
            cout << c;
            str.push_back(c);
        }
    }
}

char *getTime()
{
    time_t nowtime;
    time(&nowtime);
    nowtime = time(NULL);
    struct tm *t_set = localtime(&nowtime);
    static char str[64];
    sprintf(str, "%d-%d-%d %d:%d:%d",
            t_set->tm_year + 1900,
            t_set->tm_mon + 1,
            t_set->tm_mday,
            t_set->tm_hour,
            t_set->tm_min,
            t_set->tm_sec);
    return str;
}
