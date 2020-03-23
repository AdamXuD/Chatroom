#include "Client.h"
#include "Server.h"


/*客户端部分*/
void Client::Login() //表登录接口
{
    while (isLogin == false)
    {
        clear();
        cout << "是否拥有聊天室账户？" << endl;
        cout << "1.我已拥有账户，希望登录。" << endl;
        cout << "2.我还没有聊天室账户，希望注册。" << endl;
        switch (input())
        {
        case 1:
            clear();
            char password[32];
            input(this->acc.account, "请输入用户名：");
            strcpy(password, getpass("请输入密码："));
            strcpy(this->acc.pwd, crypt(password, password));
            Login(this->acc);
            break;
        case 2:
            Signup();
            break;
        default:
            cout << "输入错误，请重新输入！" << endl;
        }
    }
}
void Client::Login(Account acc) //重载里登录接口
{
    clear();
    cout << "登录中……" << endl;
    setMsg(msg, LOGIN, acc.account, acc.pwd, nullptr);
    if (sendMsg(msg, sock_fd) > 0) //发送登入请求
    {
        sleep(3);
        recvMsg(sock_fd, msg, true);
        if (msg.type == SUCCESS)
        {
            isLogin = true;
            cout << "登录成功！" << endl;
            cout << "是否在本地保存登陆凭据？" << endl;
            cout << "1.是" << endl;
            cout << "2.否" << endl;
            switch (input())
            {
            case 1:
            {
                fstream userinfo;
                userinfo.open("userinfo", ios::out);
                userinfo << "ACC:" << acc.account << endl;
                userinfo << "PWD:" << acc.pwd;
                userinfo.close();
                break;
            }
            case 2:
                break;
            }
        }
        else
        {
            cout << "登录失败，请检查账号或密码是否正确！" << endl;
        }
    }
    else
    {
        cout << "请求失败，请检查网络设置！" << endl;
    }
    user_wait();
    clear();
}
void Client::Signup()
{
    clear();
    char password1[32];
    char password2[32];
    input(acc.account, "请输入昵称：");
    while (1)
    {
        strcpy(password1, getpass("请输入密码："));
        strcpy(password2, getpass("请再次输入密码："));
        if (strcmp(password1, password2) == 0)
        {
            strcpy(acc.pwd, crypt(password1, password1));
            break;
        }
        else
        {
            cout << "两次密码输入不一致，请重新输入……" << endl;
            user_wait();
        }
    }
    cout << "注册中……" << endl;
    setMsg(msg, SIGNUP, acc.account, acc.pwd, nullptr);
    if (sendMsg(msg, sock_fd) > 0) //发送注册请求
    {
        sleep(1);
        recvMsg(sock_fd, msg, true);
        if (msg.type == SUCCESS)
        {
            cout << "注册成功！" << endl;
            cout << "您的用户名是：" << acc.account << endl;
            cout << "您的用户名是您登入聊天室的唯一凭据，请妥善保管您的用户名与密码！" << endl;
            user_wait();
            Login(acc);
        }
        else
        {
            cout << "请求提交失败，请检查网络设置……" << endl;
            user_wait();
        }
    }
}
void Client::fileLogin()
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
        while (isLogin == false)
        {
            clear();
            cout << "本地已有账户登录信息，是否登录？" << endl;
            cout << "1.是" << endl;
            cout << "2.否" << endl;
            switch (input())
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
                Login(acc);
                break;
            }
            case 2:
            {
                Login();
                break;
            }
            default:
                cout << "输入非法，请重新输入！" << endl;
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
        if (acc != nullptr)
        {
            onlinelist[clnt_fd].first = acc;
        }
        onlinelist[clnt_fd].second = 0;
    }
    else
    {
        onlinelist.erase(clnt_fd);
        if (acc != nullptr)
        {

            onlinelist[clnt_fd].first = acc;
        }
        onlinelist[clnt_fd].second = 0;
    }
}
void Server::Onlineremind(int call)
{
    setMsg(send_msg, ALL, ADMIN, nullptr, "Someone is Online.");
    BroadcastMsg(call, send_msg);
}
void Server::Login(int call) //登录处理函数
{
    char query[5120];
    cout << "A connector is trying to login." << endl;
    strcpy(acc.account, recv_msg.fromUser);
    strcpy(acc.pwd, recv_msg.toUser);
    sprintf(query, "select account, pwd from userinfo where account='%s';", acc.account);
    Mysql_query(&mysql, query);
    MYSQL_RES res;
    MYSQL_ROW row;
    res = *mysql_store_result(&mysql);
    row = mysql_fetch_row(&res);
    if (row != NULL)
    {
        if (strEqual(acc.pwd, row[1]))
        {
            setMsg(send_msg, SUCCESS, nullptr, nullptr, nullptr);
            sendMsg(send_msg, call); //服务端反馈
            cout << "Login success." << endl;
            addonlinelist(call, acc.account);
            cout << "Now there are " << onlinelist.size() << " user(s) online." << endl;
            Onlineremind(call);
        }
        else
        {
            setMsg(send_msg, FAILED, nullptr, nullptr, "登录失败，请检查账号或密码是否正确！");
            sendMsg(send_msg, call); //服务端反馈
            cout << "Login failed.Info does not match." << endl;
        }
    }
    else
    {
        setMsg(send_msg, FAILED, nullptr, nullptr, "登录失败，请检查账号或密码是否正确！");
        sendMsg(send_msg, call); //服务端反馈
        cout << "Login failed.Info does not match." << endl;
    }
}
void Server::Signup(int call)
{
    char query[5120];
    cout << "A connector is trying to signup." << endl;
    strcpy(acc.account, recv_msg.fromUser);
    strcpy(acc.pwd, recv_msg.toUser);
    sprintf(query, "insert into userinfo values ('%s', '%s');", acc.account, acc.pwd);
    if (Mysql_query(&mysql, query) == 0) //将用户名密码等数据写入数据库
    {
        setMsg(send_msg, SUCCESS, nullptr, nullptr, nullptr);
        sendMsg(send_msg, call); //服务端反馈
        cout << "Signup success." << endl;
    }
    else
    {
        setMsg(send_msg, FAILED, nullptr, nullptr, "注册失败，用户名已存在或服务器错误！");
        sendMsg(send_msg, call); //服务端反馈
    }
}