#import "@preview/finite:0.3.0": automaton
#import "@preview/fletcher:0.4.4": *

#set text(
  size: 12pt,
  font: ("STIX Two Text", "Source Han Serif SC"),
  lang: "zh",
)
// #set heading(numbering: "一、")
#show heading: it => {
  it
  v(-0.6em)
  box()
}
#set par(justify: true, first-line-indent: 2em, leading: 1em)
#show link: underline
#show link: set text(blue)

项目同步放在 #link("https://github.com/meoowny/Elevator_emulator")[GitHub] 上。

= 项目背景分析

项目将实现一个电梯系统的模拟，以体会操作系统的调度过程，并学习特定环境下多线程编程的方法。

该电梯系统共有二十层楼和五部电梯（可设置），当乘客按下电梯按钮时，就创建了一个电梯移动的任务；五部运行的电梯则相当于处理器，用于处理产生的任务。我们需要采用合适的调度策略来完成所有任务。

= 项目简介

== 项目结构介绍

- `assets/`：项目资源文件，主要是应用图标；
- `src/`：项目源码及 UI 文件；
- `dist/`：项目可执行文件及其运行环境；
- `xmake.lua`：项目构建文件。

== 界面说明

- 主界面：项目运行后进入该界面，可以选择需要模拟的楼层数与电梯数量；
- 电梯界面：包括左侧各个电梯内的按键与最右侧的电梯外键；
- 电梯状态显示：各电梯内按键上方显示该电梯当前所在楼层、电梯运行方向与电梯状态（由楼层显示左右的颜色指示），运行状态说明如下：
    / 灰色: 电梯正常且空闲，处于关门状态；
    / 绿色: 电梯正常且处于开门状态；
    / 橙色: 电梯正常且正在移动；
    / 红色: 电梯损坏，等待修复。

== 功能说明

- 进入程序主界面后，选择需要模拟的楼层数与电梯数量，点击“运行”开始模拟；
- 点击电梯内任意数字按键发出请求，电梯在正常状态下将移动至请求楼层；
- 电梯空闲状态下，点击开门按键可使电梯开门一段时间。若重复按开门键，开门时间不会累积；
- 电梯开门状态下，点击关门按键可使电梯直接关门，若有目标楼层正在等待则移动至目标楼层，若无目标楼层则进入空闲状态；
- 任意状态下，点击报警按键会使电梯进入损坏状态。电梯损坏时，再次点击报警按键恢复（该功能仅为方便电梯的模拟而设置）。

= 开发环境

- 操作系统：Windows11
- 开发语言：C++17
- 构建工具：xmake
- 编译与链接工具：VS2022
- GUI 框架：Qt6.6.3

= 运行方式

该项目使用 windeployqt 打包了所需的动态链接库，双击运行 dist 目录下的 osLab1.exe 即可开始运行程序。

= 项目架构

项目由两个主要的类组成：`Elevator` 与 `Controller`。

其中 `Elevator` 包含了单个电梯的界面与运行逻辑，`Controller` 由 `Elevator` 数组与电梯外按键组成。

此外 `ElevatorButton` 类可以创建电梯按键，用于绑定电梯运行事件相关的信号与槽；`Semaphore` 是使用 STL 库的互斥量和条件变量实现的信号量，用于实现线程间互斥；`MainWindow` 仅用于创建程序初始界面，进行电梯模拟系统的设置。

== 类设计

=== Elevator

=== Controller

== 调度算法设计

= 运行展示

#figure(
  image("screenshots\startup.png", width: 40%),
  caption: [程序初始界面],
) <fig-startup>
#figure(
  image("screenshots\screenshots_01.png", width: 80%),
  caption: [系统初始状态],
) <fig-screenshots_01>
#figure(
  image("screenshots\screenshots_02.png", width: 80%),
  caption: [所有电梯正常运行时],
) <fig-screenshots_02>
#figure(
  image("screenshots\screenshots_03.png", width: 80%),
  caption: [部分电梯损坏时],
) <fig-screenshots_03>

// #automaton((
//         W: (R: "数字键"),
//         R: (W: "到达楼层"),
// ))

// #diagram(
//   node-stroke: .1em,
//   // node-fill: gradient.radial(blue.lighten(80%), blue, center: (30%, 20%), radius: 80%),
//   spacing: 4em,
// 
//   edge((-1, 0), "r", "-|>", [电梯对象创建], label-pos: 0, label-side: center),
//   node((0,0), `WAITING`, radius: 2em),
//   edge(`onNewTarget`, "-|>"),
//   node((1.5,0), `RUNNING`, radius: 2em),
//   edge(`arrive()`, "-|>"),
//   node((3,0), `OPENED`, radius: 2em, extrude: (-2.5, 0)),
//   node((1.5,1.5), `BROKEN`, radius: 2em, extrude: (-2.5, 0)),
// 
//   edge((0,0), (3,0), [按下楼层键/开门键], "-|>", bend: 40deg),
//   edge((3,0), (0,0), `close()`, "-|>", bend: 40deg),
// )

