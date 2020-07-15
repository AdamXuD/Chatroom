# 聊天室Chatroom

这是一个简单易用的聊天室练手项目。

## 简介

这是一个聊天室的练手项目，旨在提高对C/C++与网络编程的理解，本聊天室虽然只是运行在shell上的命令行程序，但还是力求提高其易用性，用户体验。让C/C++初学者，使用者摆脱命令行界面简陋，交互体验差等死板固有印象。

本程序客户端服务端一体。对服务端有环境要求（主要是数据库），客户端没有环境要求（可能需要安装dl库），引入了`jsoncpp`，`sqlite3`等第三方库。

## 运行环境

目前测试在Ubuntu 18.04下使用没有问题。

## 编译流程

在项目根目录下，先`cmake`生成`makefile`再make进行编译

```shell
cd Chatroom-master
mkdir build
cd build
cmake ..
make
```

编译要求服务端必须要有`mysql-devel`，若打算将数据库假设在本地，则要求要有`mysql-server`，`mysql-client`等软件。

### CentOS下MySQL 5.7的安装

参考这篇博客：https://www.cnblogs.com/zsh-blogs/p/11497720.html

### Ubuntu下MySQL 5.7的安装

参考这篇博客：https://blog.csdn.net/HYESC/article/details/100074665

## 使用教程

### 服务端

第一次运行服务端，请先初始化服务端。

安装完MySQL后请为聊天室专门准备一个空的数据库，安全起见也请准备一个新的数据库用户（不准备也行，可以直接用root账户）。

然后在程序目录下执行

```shell
./Chatroom --init
```

之后会看到如下提示：

```
Didn't find config.json. A new config.json has been created.
Please finish the information in config.json first.
After you finish the config, please input `confirm` to go ahead.
```

此时会在程序目录下生成一个`config.json`，内容如下：

```json
{
   "Client" : {
      "Account" : null,
      "Port" : 0,
      "ServerIP" : "null",
      "pwd" : null
   },
   "Server" : {
      "DatabaseAccount" : null,
      "DatabaseName" : null,
      "DatabasePassword" : null,
      "DatabasePort" : null,
      "ListenIP" : "null",
      "Port" : 0
   }
}
```

运行服务器时仅需把`Server`的数据全部填上即可。

| Server下的字段名 | 含义                                           |
| ---------------- | ---------------------------------------------- |
| DatabaseAccount  | 为聊天室新建的数据库账号（没建立则可以为root） |
| DatabaseName     | 为聊天室新建的数据库名称                       |
| DatabasePassword | 上述数据库账号对应的密码                       |
| DatabasePort     | 数据库端口号（没改过的话默认为3306）           |
| ListenIP         | 聊天室监听的IP（127.0.0.1）                    |
| Port             | 聊天室监听的端口号                             |

输入完成以后在程序里输入`confirm`并回车，之后程序会初始化聊天室所需的数据库，出现一下提示则代表初始化成功。

```
Success to connect database
Creating `userinfo` table...
Creating `groupinfo` table...
Creating `history` table...
Creating `friendlist` template table...
Creating `group` template table...
Creating `querybox` template table...
Initialization is finished, please restart without argument.
```

若初始化失败，将会提示错误信息，请根据错误信息自行查找并修复错误。

之后无参数运行聊天室，选择服务端后，它会自动按照`json`配置并监听相应IP与端口。

### 客户端

**客户端我们尽可能设计的简单化易用化了，关于具体操作没什么好说的。**

启动客户端请直接无参数运行程序，并用上下键选择客户端。

第一次运行客户端或者`config.json`中没有填写`ServerIP`与`Port`的话程序将会引导填写`IP`与`Port`并保存在`config.json`中，下一次则无需填写。

填写完以后客户端则会进行连接，请确保客户端连接前服务端已准备好并已进入监听状态。

连接成功后进入登录注册步骤。

```
是否拥有聊天室账户？
>我已拥有账户，希望登录。
>我还没有聊天室账户，希望注册。
```

此处也是菜单选择，未注册则必须先注册。

注册时输入账号与密码，之后注册请求将会被发送至服务端处理。

**请放心，密码将会被Linux下自带的`crypt`库不可逆加密后再发送，不会导致个人信息泄露。**

注册完成后自动进行登录步骤，登录完成后会提示是否保存密码。

**这也请放心，选择保存账号密码后，账号以及不可逆加密后的密码将会被保存在`config.json`中，这也不会导致个人信息泄露。**

下一次登陆前程序会检索文件中是否有保存账号密码，有的话可以进行**免密登录**。

登录成功后则进入全局广播模式，此时可以输入`/menu`并回车唤出菜单，菜单内容如下：

```
>发起私聊
>发起群聊
>添加好友
>删除好友
>获取请求
>处理请求
>设置好友为特别关心
>设置好友为黑名单好友
>请求好友列表
>获取在线好友列表
>创建群组
>加入群组
/*...*/
>返回
```

用户可以做的操作如上所示，当发起私聊，群聊请求后，聊天的目标对象（某个用户或群），发来的消息将会**高亮**显示。

如果想对群组群员操作，则需要进入群聊系统后，使用`/menu`调出其子菜单进行操作。

**只有在广播与群聊界面能通过`/menu`调出相应的子菜单，其他界面则不能主动调出子菜单。**

对好友以及群员进行操作时，会专门有一个**群员菜单或好友菜单**便于用户选择，控制。

**在此程序中，几乎所有的操作都会用菜单来进行，除了`/menu`需要用命令行进行，这也是我们为了尽可能改善用户体验，使用户摆脱对于命令行操作的简陋，死板，繁琐的固有印象。所以在此程序中，除了`/menu`和发送信息操作等以及一部分必须要向用户收集的信息等需要用户主动输入以外，其他操作无一例外只需要用户用菜单进行，十分的简便。**

用户所有的聊天记录，好友列表，群员列表则会保存在程序当前目录下的`你的用户名.db`文件中，这是个数据库文件，由`sqlite3`库创建，可用相应的工具打开并操作。

~~后期可能会引入相应的小工具来帮助用户读取内部信息~~

## 关于Bug以及该项目的维护

若是有什么严重的bug请直接提交issue。

