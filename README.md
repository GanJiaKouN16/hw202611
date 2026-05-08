# 魔兽世界终极版

## 项目概述

- **功能**: 模拟红蓝两军完整战斗系统，按时序输出所有事件

## 项目结构

```
a10/
├── warcraft.cpp    # 主程序源码
├── AGENTS.md       # 项目架构文档
├── rules.txt       # 题目规则说明
├── test.txt        # 样例输入输出
└── README.md       # 本文件
```

## 核心实现

### 类设计

| 类名 | 说明 |
|------|------|
| `Weapon` | 武器类（Sword/Bomb/Arrow） |
| `Warrior` | 武士基类 |
| `Dragon` | 龙武士 - 士气属性，胜利后欢呼 |
| `Ninja` | 忍者武士 - 两件武器，不反击 |
| `Iceman` | 冰人武士 - 每走两步生命-9攻击+20 |
| `Lion` | 狮子武士 - 忠诚度，战死转移生命值 |
| `Wolf` | 狼武士 - 获胜缴获武器 |
| `City` | 城市类 - 生命元、旗帜、战斗历史 |
| `Headquarter` | 司令部类 - 生命元、造兵序列 |

### 时序引擎

| 时间 | 事件 |
|------|------|
| :00 | 武士降生 |
| :05 | Lion 逃跑 |
| :10 | 武士前进 |
| :20 | 城市产出生命元 |
| :30 | 单武士收取生命元 |
| :35 | Arrow 放箭 |
| :38 | Bomb 自爆评估 |
| :40 | 战斗（攻击/反击/死亡/欢呼/缴获/旗帜） |
| :50 | 司令部报告生命元 |
| :55 | 武士报告武器 |

### 造兵顺序

- **红方**: iceman → lion → wolf → ninja → dragon
- **蓝方**: lion → dragon → ninja → iceman → wolf

## 编译与运行

### 编译

```bash
g++ -o warcraft.exe warcraft.cpp -std=c++11 -O2
```

### 运行

```bash
# 从文件输入
./warcraft.exe < input.txt > output.txt

# 或使用样例测试
./warcraft.exe < test.txt
```

### 验证

```bash
diff output.txt expected.txt
```

## 关键约定

1. **时间格式**: `printf("%03d:%02d", hour, minute)` - 三位时两位分
2. **生命值**: ≤ 0 时视为 0，不是负数
3. **Sword 攻击力**: 武士攻击力 × 20% 向下取整，每次战斗后变为 80%
4. **Arrow 攻击力**: 固定为 R，使用 3 次后消失
5. **Iceman 每走 2 步**: 生命值 -9（若 ≤ 0 则变为 1），攻击力 +20
6. **Lion 战死转移**: 战斗前的生命值（:40 分前一瞬间）
7. **司令部奖励**: 先奖励所有战斗胜利者（优先离敌方近的），再回收城市生命元
8. **同归于尽**: 不算战斗，不拿城市生命元，不影响旗帜
9. **旗帜规则**: 连续两场同一方胜利才升旗（中间有平局不算连续）
10. **输出顺序**: 同时间事件按地点从西向东；战斗中事件按编号 6-11 顺序输出

## 样例验证

**输入：**
```
1
20 1 10 10 1000
20 20 30 10 20
5 5 5 5 5
```

**关键输出检查点：**
- `000:00 blue lion 1 born` + loyalty=10
- `000:10 blue lion 1 marched to city 1`
- `001:00 blue dragon 2 born` + morale=0.00
- `001:10 blue lion 1 reached red headquarter` → 占领计数=1
- `002:10 blue dragon 2 reached red headquarter` → 占领计数=2 → 战争结束

## 常见陷阱

| 陷阱 | 说明 |
|------|------|
| 时间格式 | `printf("%03d:%02d", hour, minute)` 三位时两位分 |
| 造兵等待 | 生命元不足时跳过当前轮次，等够了再按序列下一个 |
| arrow 互射 | 相邻城市可能同时射死对方 |
| bomb 时机 | :38 分评估，:40 分战斗前判断是否会死 |
| 输出截断 | 战争结束后不再输出任何事件，但同时间的事件全部输出 |

## 实现步骤

### 1. 基础框架搭建
- 定义枚举类型：WarriorType、WeaponType、Color
- 定义常量：造兵顺序、武士名称、武器名称

### 2. 武器系统实现
- Weapon 结构体：type、attack、use_count
- Sword：攻击力 = 武士攻击力 × 20%，每次战斗后 × 80%
- Bomb：一次性使用，同归于尽
- Arrow：固定攻击力 R，使用 3 次后消失

### 3. 武士系统实现
- Warrior 基类：id、life、force、position、weapons
- Dragon：morale 属性，胜利后欢呼
- Ninja：两件武器（sword + bomb），不反击
- Iceman：steps 计数器，每走 2 步变化
- Lion：loyalty 属性，战死转移生命值
- Wolf：获胜后缴获敌人武器

### 4. 城市与司令部
- City：elements、flag、consecutive_wins、warriors[2]
- Headquarter：elements、enemy_count、warrior_count、current_index

### 5. 时序引擎实现
- 按分钟推进（0-59）
- 各时间点触发对应事件处理器
- 战争结束标志：war_ended

### 6. 事件处理器实现
- event_born()：武士降生
- event_lion_escape()：Lion 逃跑
- event_march()：武士前进
- event_city_produce()：城市产出生命元
- event_collect()：收取生命元
- event_arrow()：Arrow 放箭
- event_bomb()：Bomb 自爆评估
- event_battle()：战斗处理
- event_report_elements()：报告生命元
- event_report_weapons()：报告武器

### 7. 战斗系统实现
- 确定攻击顺序（旗帜/奇偶城市）
- 主动攻击 → 反击 → 死亡判定
- Wolf 缴获武器
- Dragon 士气变化与欢呼
- Lion 忠诚度变化
- 旗帜更新

### 8. 测试与调试
- 使用样例输入验证
- 检查时间格式输出
- 验证边界情况（M=1, N=1, T=0）
