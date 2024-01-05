# 配置文件说明
```json
{
  "game_max_round": 1200,// 游戏最大回合数
  "game_print_map": true,// 是否打印地图到控制台
  "game_print_map_ascii": false,// 打印地图是否打印ascii
  "game_snapshot": true,// 是否开启快照功能
  "map_size": 13,// 地图尺寸大小,5-30
  "player_num": 2,// 玩家数量,1-4
  "player_hp": 1,// 玩家初始化hp
  "player_max_hp": 3,// 玩家最大的hp
  "player_speed": 2,// 玩家初始化速度
  "bomb_time": 3,// 炸弹的爆炸时间
  "bomb_num": 2,// 每个人初始化炸弹数量
  "bomb_range": 1,// 炸弹的炸弹范围
  "bomb_random": 1,// 炸弹随机数范围
  "shield_time": 30,// 护盾时间
  "invincible_time": 15,// 无敌时间
  "mark_kill": 10000,// 击杀加分
  "mark_dead": 12000,// 死亡扣分
  "mark_pick_potion": 100,// 捡道具得分
  "mark_bomb_mud": 10,// 炸泥块得分
  "potion_probability": 50,// 泥块产生道具概率
  "mud_num": 70,// 泥块数量
  "wall_random": 25,// 墙壁随机生成概率
  "seed_random": 0,// 随机数种子
  "result_path": "result.json",// 结果文件位置
  "black_list":[],// 黑名单玩家名

  "timer_initial_value": 500,// 定时器间隔
  "round_interval_value": 500,// 回合间隔
  "server_max_connection_num": 10,// 服务器最大连接数量
  "epoll_max_events_num": 100,// epoll最大event
  "epoll_timeout": 10,// epoll超时
  "log_print_stdout": false,// 是否打印日志到控制台

  "host": "0.0.0.0",// 绑定ip
  "port": 9999// 绑定端口号
}
```
