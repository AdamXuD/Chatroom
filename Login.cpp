#include "Client.h"
#include "Server.h"

extern char content[5120];
extern char query[10240];

/*客户端部分*/
void Client::Login() //表登录接口
{
    while (isLogin == false)
    {
        clear();
        string list[2] = {"我已拥有账户，希望登录。", "我还没有聊天室账户，希望注册。"};
        switch (menu(list, 2, "是否拥有聊天室账户？"))
        {
        case 1:
            clear();
            char password[32];
            input(this->acc.account, "请输入用户名：");
            strcpy(password, Getpass("请输入密码："));
            strcpy(this->acc.pwd, crypt(password, "pa"));
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
            string list[2] = {"是", "否"};
            switch (menu(list, 2, "登录成功！\n是否在本地保存登陆凭据？"))
            {
            case 1:
            {
                Json::Value root;
                Json::Reader reader;

                fstream config("config.json", ios::in);
                reader.parse(config, root);
                config.close();

                root["Client"]["Account"] = Json::Value(acc.account);
                root["Client"]["pwd"] = Json::Value(acc.pwd);

                Json::StyledWriter writter;
                config.open("config.json", ios::out);
                config << writter.write(root);
                config.close();
                break;
            }
            case 2:
                break;
            }
            databaseInit();
        }
        else
        {
            cout << "登录失败，请检查账号或密码是否正确！" << endl;
            user_wait();
        }
    }
    else
    {
        cout << "请求失败，请检查网络设置！" << endl;
        user_wait();
    }
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
        strcpy(password1, Getpass("请输入密码："));
        strcpy(password2, Getpass("请再次输入密码："));
        if (strcmp(password1, password2) == 0)
        {
            strcpy(acc.pwd, crypt(password1, "pa"));
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
    Json::Value root;
    Json::Reader reader;

    fstream config("config.json", ios::in);
    reader.parse(config, root);
    config.close();

    if (root["Client"]["Account"].isNull() || root["Client"]["pwd"].isNull())
    {
        Login();
    }
    else
    {
        while (isLogin == false)
        {
            clear();
            string list[2] = {"1.是", "2.否"};
            switch (menu(list, 2, "本地已有账户登录信息，是否登录？"))
            {
            case 1:
            {
                strcpy(acc.account, root["Client"]["Account"].asCString());
                strcpy(acc.pwd, root["Client"]["pwd"].asCString());
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
    map<int, pair<string, int>>::iterator it;
    for (it = onlinelist->begin(); it != onlinelist->end(); it++)
    {
        if (strEqual(it->second.first, acc))
        {
            setMsg(send_msg, FORCE_EXIT, ADMIN, it->second.first.c_str(), "您的账号已在别处登录！");
            sendMsg(send_msg, it->first);
            onlinelist->erase(it->first);
        }
    }
    if (onlinelist->count(clnt_fd) == 0)
    {
        if (acc != nullptr)
        {
            (*onlinelist)[clnt_fd].first = acc;
        }
        (*onlinelist)[clnt_fd].second = 0;
    }
    else
    {
        onlinelist->erase(clnt_fd);
        if (acc != nullptr)
        {
            (*onlinelist)[clnt_fd].first = acc;
        }
        (*onlinelist)[clnt_fd].second = 0;
    }
}
void Server::Onlineremind(int call)
{
    setMsg(send_msg, ALL, ADMIN, nullptr, "Someone is Online.");
    BroadcastMsg(call, send_msg);
}
void Server::Login(int call) //登录处理函数
{
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
            cout << "Now there are " << onlinelist->size() << " user(s) online." << endl;
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
    cout << "A connector is trying to signup." << endl;
    strcpy(acc.account, recv_msg.fromUser);
    strcpy(acc.pwd, recv_msg.toUser);
    sprintf(query, "insert into userinfo values ('%s', '%s');", acc.account, acc.pwd);
    if (Mysql_query(&mysql, query) == 0) //将用户名密码等数据写入数据库
    {
        setMsg(send_msg, SUCCESS, nullptr, nullptr, nullptr);
        sendMsg(send_msg, call); //服务端反馈
        cout << "Signup success." << endl;
        createFriendList();
        createQuerybox();
    }
    else
    {
        setMsg(send_msg, FAILED, nullptr, nullptr, "注册失败，用户名已存在或服务器错误！");
        sendMsg(send_msg, call); //服务端反馈
    }
}