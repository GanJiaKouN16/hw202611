# AGENTS.md

## 1. 项目概述

本项目是北京大学 Online Judge (POJ) 经典编程题 **"魔兽世界终极版"** 的解题代码。题目要求模拟红蓝两军的完整战斗系统，按时序输出所有事件。技术栈为 C++（推荐）或 Python，无第三方依赖。代码结构为单文件实现，输入从 stdin 读取，输出到 stdout。

## 2. 快速命令

| 操作 | 命令 |
|------|------|
| 编译 C++ | `g++ -o warcraft warcraft.cpp -std=c++11 -O2` |
| 运行（单组测试） | `echo "1\n20 1 10 10 1000\n20 20 30 10 20\n5 5 5 5 5" \| ./warcraft` |
| 运行（从文件） | `./warcraft < input.txt` |
| 对拍验证 | `diff output.txt expected.txt` |
| Python 运行 | `python warcraft.py < input.txt` |

## 3. 核心架构（单文件）

```
warcraft.cpp
├── 常量定义        // 武士类型、武器类型、事件时间点
├── 类定义
│   ├── Weapon      // 武器：sword/bomb/arrow，含攻击力与使用次数
│   ├── Warrior     // 武士：编号、生命值、攻击力、武器、位置
│   ├── Dragon      // 继承 Warrior：士气属性，胜利后欢呼
│   ├── Ninja       // 继承 Warrior：两件武器，不反击
│   ├── Iceman      // 继承 Warrior：每走两步生命-9攻击+20
│   ├── Lion        // 继承 Warrior：忠诚度，战死转移生命值
│   ├── Wolf        // 继承 Warrior：获胜缴获武器
│   ├── City        // 城市：生命元、旗帜、战斗历史
│   └── Headquarter // 司令部：生命元、造兵序列、敌军计数
├── 时序引擎        // 按分钟推进，按时间点触发事件
├── 事件处理器
│   ├── :00  武士降生
│   ├── :05  Lion 逃跑
│   ├── :10  武士前进
│   ├── :20  城市产出生命元
│   ├── :30  单武士收取生命元
│   ├── :35  Arrow 放箭
│   ├── :38  Bomb 自爆评估
│   ├── :40  战斗（攻击/反击/死亡/欢呼/缴获/旗帜）
│   ├── :50  司令部报告生命元
│   └── :55  武士报告武器
└── 输出格式化      // 时:分 三位时两位分
```

## 4. 关键数据结构

```cpp
enum WarriorType { DRAGON, NINJA, ICEMAN, LION, WOLF };
enum WeaponType  { SWORD, BOMB, ARROW };

// 红方造兵顺序: iceman → lion → wolf → ninja → dragon
// 蓝方造兵顺序: lion → dragon → ninja → iceman → wolf
const int RED_ORDER[5]  = {2, 3, 4, 1, 0};
const int BLUE_ORDER[5] = {3, 0, 1, 2, 4};
```

## 5. 关键约定（违反会导致 WA）

1. **生命值 <= 0 时视为 0**，不是负数
2. **sword 攻击力 = 武士攻击力 * 20% 向下取整**，每次战斗后变为原来的 80% 向下取整
3. **arrow 攻击力固定为 R**，使用 3 次后消失，不能攻击司令部内的敌人
4. **iceman 每走 2 步**：生命值 -9（若 -9 后 <= 0 则变为 1），攻击力 +20
5. **lion 战死时转移的是战斗前的生命值**（:40 分前一瞬间），不是死后的
6. **司令部奖励顺序**：先奖励所有战斗胜利者（优先离敌方近的），再回收城市生命元
7. **同归于尽不算战斗**：不拿城市生命元，不影响旗帜
8. **旗帜规则**：连续两场同一方胜利才升旗（中间有平局不算连续）
9. **输出顺序**：同时间事件按地点从西向东；战斗中事件按编号 6-11 顺序输出
10. **到达司令部的武士不再移动**，但司令部内敌人达到 2 个时战争结束

## 6. 验证流程

```
1. 编译:   g++ -o warcraft warcraft.cpp -std=c++11
2. 输入:   使用 test.txt 中的样例输入
3. 运行:   ./warcraft < input.txt > my_output.txt
4. 对拍:   diff my_output.txt expected.txt
5. 边界:   测试 M=1, N=1, T=0 等极端情况
```

## 7. 样例验证

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

## 8. 常见陷阱

| 陷阱 | 说明 |
|------|------|
| 时间格式 | `printf("%03d:%02d", hour, minute)` 三位时两位分 |
| 造兵等待 | 生命元不足时跳过当前轮次，等够了再按序列下一个 |
| arrow 互射 | 相邻城市可能同时射死对方 |
| bomb 时机 | :38 分评估，:40 分战斗前判断是否会死 |
| 输出截断 | 战争结束后不再输出任何事件，但同时间的事件全部输出 |
