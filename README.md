#说明与要求
1.项目里一共有三个branch 你们默认是用develop这条branch的
  剩下的master和release是被保护状态留作备份和发布用的
  当develop没有明显错误的话每隔一段时间我就会将develop备份到master里
2.老生常谈：
  **开始操作自己的工作区前要从服务器上同步（pull）代码下来（不然你push的时候容易报conflict）**
  **在上传（push）要先自己试跑一遍代码，能编译通过没有什么致命问题再push到服务器上（要是别人pull你的代码下来编译不过报错的话会很麻烦）**
  **提交修改的时候记得写自己做了什么操作**
3.下面五个常用（每次都会用到）的指令务必记住
  git pull
  git add ...
  git commit -m "...（修改内容）"
  git push
4.大概没什么要强调的了
http://www.imooc.com/wenda/detail/521148