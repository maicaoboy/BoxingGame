# Linux系统级开发的网络拳击游戏
- 描述
  游戏中选手可以通过按A和D键左右移动，按I和O键左右格挡，K和L建进行左右攻击。其中左格挡可以抵消左侧敌人的右侧攻击，右侧格挡可以抵消右侧敌人的左侧攻击，其它状态均不可以抵消攻击。每名选手初始生命值为5，每次受到攻击损失1点生命值，生命值为0即死亡。
- 总体设计
  在服务器和每个客户端都存放一个玩家数组，使用多进程及I/O复用并设计通信协议完成玩家数组的同步，并在每次通信后使用curses展示最新画面。
- 服务端
  服务器端采用单线程并使用epoll进行I/O复用(单线程)，以处理多用户的游戏并发。
- 客户端
  客户端的设计：主进程负责发送数据和接收用户输入，并负责键盘操作部分的画面展示工作，另外使用一个新的线程接受来自服务器的数据，并负责服务器数据到本地数据的同步及此部分数据到画面的刷新。
- 游戏截图
  ![image text](https://github.com/maicaoboy/BoxingGame/blob/main/screenshot/%E6%B8%B8%E6%88%8F%E6%88%AA%E5%9B%BE1.png)
  ![image text](https://github.com/maicaoboy/BoxingGame/blob/main/screenshot/%E6%B8%B8%E6%88%8F%E6%88%AA%E5%9B%BE2.png)
  ![image text](https://github.com/maicaoboy/BoxingGame/blob/main/screenshot/%E6%B8%B8%E6%88%8F%E6%88%AA%E5%9B%BE3.png)
