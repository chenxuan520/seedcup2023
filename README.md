# seedcup2023 server

![](https://raw.githubusercontent.com/Thyme-git/seedcup2023-client/main/image/2023%E7%A7%8D%E5%AD%90%E6%9D%AF%E8%B5%9B%E9%A2%98/1699091546594.png)

## 作者

- [chenxuan](https://github.com/chenxuan520)

- [thyme](https://github.com/Thyme-git)

## 目录说明
```shell

.
├── LICENSE
├── README.md
└── src
    ├── CMakeLists.txt
    ├── config.json
    ├── server
    │   ├── CMakeLists.txt
    │   ├── game
    │   │   ├── action.h
    │   │   ├── area.h
    │   │   ├── block
    │   │   │   ├── block_base.h
    │   │   │   ├── block_factory.h
    │   │   │   ├── mud.h
    │   │   │   └── wall.h
    │   │   ├── bomb.h
    │   │   ├── const.h
    │   │   ├── custom_map.h
    │   │   ├── game.cpp
    │   │   ├── game.h
    │   │   ├── player.h
    │   │   ├── potion
    │   │   │   ├── gloves_potion.h
    │   │   │   ├── invincible_potion.h
    │   │   │   ├── num_potion.h
    │   │   │   ├── potion_base.h
    │   │   │   ├── potion_factory.h
    │   │   │   ├── range_potion.h
    │   │   │   ├── rebirth_potion.h
    │   │   │   ├── shield_potion.h
    │   │   │   └── speed_potion.h
    │   │   ├── print.h
    │   │   └── snapshot.h
    │   ├── main.cpp
    │   ├── net
    │   │   ├── api.cpp
    │   │   ├── api.h
    │   │   ├── server.cpp
    │   │   └── server.h
    │   ├── README.md
    │   ├── snapshot_main.cpp
    │   └── term_main.cpp
    ├── test
    │   ├── main
    │   ├── main.cpp
    │   ├── Makefile
    │   ├── README.md
    │   ├── snapshot_test.h
    │   └── test.h
    └── util
        ├── config.cpp
        ├── config.h
        ├── json.hpp
        ├── logger.h
        └── random.h
```

- game中为chexuan开发,net中为thyme开发,因此可能出现规范和开发习惯的差异

- game为游戏内核,负责处理游戏的判定和运行

- net为处理网络连接层,负责连接维护,数据接收转换

- term_main.cpp为无网络net版本的测试server,main.cpp为入口文件

- util装工具包,json库使用的是[https://github.com/nlohmann/json](https://github.com/nlohmann/json)

## 编译方式
> 需要linux环境,最好是ubuntu或者debain系列的,需要装有C++的环境,即(g++,cmake,make等)

1. `sudo apt install -y libfmt-dev libspdlog-dev`,安装所依赖的日志库

2. `cd src;mkdir build;cd build;cmake ..;make`,编译项目

3. 没问题的话会在src目录下生成bin目录,目录中存在server和term两个二进制文件

## 关联仓库

- [赛题以及python客户端仓库](https://github.com/Thyme-git/seedcup2023-client)

- [cppsdk以及bot仓库](https://gitee.com/chenxuan520/seedcup-cppsdk.git),bot在[feat/bot分支下](https://gitee.com/chenxuan520/seedcup-cppsdk/tree/feat%2Fbot/)

- [gosdk仓库](https://github.com/chenxuan520/seedcup-gosdk)

## TODO

- [x] ~~添加地图的初始化设置功能,方便测试~~

- [x] ~~增加快照功能方便回溯~~

- [ ] 完善自动化测试,使用github-cicd
