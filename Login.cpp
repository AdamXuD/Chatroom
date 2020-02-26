#include "Client.h"
#include "Server.h"

/*客户端部分*/
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
    user_wait();
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
/*客户端部分*/

/*服务器部分*/
void Server::addonlinelist(int clnt_fd, char *acc)
{
    if (onlinelist.count(clnt_fd) == 0)
    {
        onlinelist[clnt_fd].first = acc;
        onlinelist[clnt_fd].second = 0;
    }
    else
    {
        onlinelist.erase(clnt_fd);
        onlinelist[clnt_fd].first = acc;
        onlinelist[clnt_fd].second = 0;
    }
}
void Server::Onlineremind(int call)
{
    memset(&msg, 0, sizeof(msg));
    msg.type = ALL;
    strcpy(msg.fromUser, "Admin");
    strcpy(msg.content, "Someone is Online.");
    BroadcastMsg(call);
}
void Server::Login(int call) //登录处理函数
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
                addonlinelist(call, acc.account);
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
        char tmp[1024] = {0};
        strcpy(tmp, "FAILED:Login failed.Database error:");
        strcat(tmp, mysql_error(&mysql));
        memset(&msg, 0, sizeof(msg));
        msg.type = COMMAND;
        strcpy(msg.content, tmp);
        sendMsg(msg, call); //服务端反馈
        cout << tmp << endl;
    }
}
void Server::Signup(int call)
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
    if (mysql_query(&mysql, query) == 0) //将用户名密码等数据写入数据库
    {
        memset(&msg, 0, sizeof(msg));
        msg.type = COMMAND;
        strcpy(msg.content, "SUCCESS");
        sendMsg(msg, call); //服务端反馈
        cout << "Signup success." << endl;
        createFriendList();
        createQuerybox();
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