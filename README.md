# 项目介绍
这是个利用PD协议实现的恒温加热台，主控采用CW32F030，这个是国产32，有幸免费样品申请，申请到了所有用其作为主控制作一个项目。项目主要借鉴来源于[[PD协议 | 高颜值\]mini加热台 - 嘉立创EDA开源硬件平台 (oshwhub.com)](https://oshwhub.com/littleoandlittleq/bian-xie-jia-re-tai)

# 软件架构
开发环境 VScode+gcc编译开发环境，

# 制作过程记录

## 2023年7月26日

基础工程模板搭建，采用easylogger日志系统，便于log调试。整体工程采用VScode+GCC搭建开发环境，jlink调试。

![img](pic/1.png)

## 2023年7月27日

PWM部分调试完成，可以设置不同频率，占空比的PWM。

![1690470426680](pic/1690470426680.png)

## 2023年7月29日

完成外部中断，adc部分调试，规划设计路线

![1690989359742](pic/1690989359742.png)

## 2023年8月3日

调通oled外设，ec11按键逻辑编写完成

![1691070784430](pic/1691070784430.png)

## 2023年8月5日

开机过渡动画设计完成。

![ezgif.com-video-to-gif](pic/ezgif.com-video-to-gif.gif)