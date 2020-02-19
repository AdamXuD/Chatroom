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

int sendMsg(const Msg msg, int fd) //发送消息用接口
{
    char buf[65535];
    memset(buf, 0, sizeof(buf));
    memcpy(buf, &msg, sizeof(msg));
    int ret = send(fd, buf, sizeof(buf), 0);
    return ret;
}

int recvMsg(int fd, Msg &msg) //接收消息用接口
{
    char buf[65535];
    int ret;
    memset(&msg, 0, sizeof(msg));
    while (msg.type == 0) //检测是否接收到消息
    {
        usleep(1000); //减缓循环频率
        memset(buf, 0, sizeof(buf));
        ret = recv(fd, buf, 65535, 0);
        memcpy(&msg, buf, sizeof(msg));
    }
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
