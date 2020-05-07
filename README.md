# 记分牌算法

## Issue

若指令dst未被使用且功能空闲，则发射并更新
否则后续指令全卡死，直到存在空闲。
**解决 WAW**

## Read operands

如果已经发射未运算的指令的操作数不“待写”，则取数并开始运算。
**解决RAW，可能乱序**

## Excution

运算

## Write results

若之前的取数前“挂机”指令的src中不包含读该指令的dst，可写回
**解决WAR**

# Tomasulo算法

## Issue

只要功能空闲即可发射，并将操作数重命名为功能编号

## Execute

操作数都被替换成数值后可以执行，否则监听数据公交

## Write result

执行后把自己结果广播到数据公交

## 代码架构：

	main.cpp				整体运行
	sim.h/sim.cpp			实现模拟器sim类
		sim类功能：
			config(File)	输入输出文件
			init()			初始化
			run()			顺序执行
		Tomasulo类			继承sim类
			重写init()
			实现run()类

