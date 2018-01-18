##### IR_CTRL #####


| SN98601 IO口 | SN98660 IO口 | SN98661 IO口 | 设计功能 | 备注 | 
| ------------ | ------------ | ------------ | -------- | ---- |
| MS1_IO2 | MS1_IO2 | MS1_IO2 | IR CUT 驱动控制输出A |  
| MS1_IO3 | AUD_IO0 | AUD_IO0 | 预留灯板红外灯控制  |  
| MS1_IO4 | AUD_IO1 | AUD_IO1 | IR CUT 驱动控制输出B | 　
| MS1_IO5 | AUD_IO2 | AUD_IO2 | Day/Night mode 检测口 |  　
| MS1_IO6 | AUD_IO3 | AUD_IO3 | Sensor 电源控制 | H:Enable  L:Disable |
| MS1_IO7 | AUD_IO4 | AUD_IO4 | Speaker SNAP01 控制 |  H:Enable  L:Disable |
| MS1_IO8 | MS1_IO3 | MS1_IO3 | SF_CSn | 　|
| MS1_IO9 | MS1_IO4 | MS1_IO4 | SF_DO |  　|
| MS1_IO10 |MS1_IO5 | MS1_IO5 | SF_DI |  　

* 可以透過修改ir_ctrl.c macro進行IQ腳位修改

###### Usage ##########

* 使用方法：
	1. 设置白天模式：
		./snx_ir_ctl 0	
	2. 设置夜间模式：
		./snx_ir_ctl 1
	3. 检测灯板GPIO，设置IR day/night 模式		
		./snx_ir_ctl 
* 说明：检测灯板GPIO，如果有变化切换到相应的模式（day/night)

