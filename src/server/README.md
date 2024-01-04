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
  "bomb_random": 1,
  "shield_time": 30,
  "invincible_time": 15,
  "mark_kill": 10000,
  "mark_dead": 12000,
  "mark_pick_potion": 100,
  "mark_bomb_mud": 10,
  "potion_probability": 50,
  "mud_num": 70,
  "wall_random": 25,
  "seed_random": 0,
  "result_path": "result.json",
  "black_list":[],

  "timer_initial_value": 500,
  "round_interval_value": 500,
  "server_max_connection_num": 10,
  "epoll_max_events_num": 100,
  "epoll_timeout": 10,
  "log_print_stdout": false,

  "host": "0.0.0.0",
  "port": 9999
}
```
