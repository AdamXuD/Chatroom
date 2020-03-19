#include "Common.h"

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
    memset(&msg, 0, sizeof(msg));
    return ret;
}

int recvMsg(int fd, Msg &msg) //接收消息用接口
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
    }
    return ret;
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
