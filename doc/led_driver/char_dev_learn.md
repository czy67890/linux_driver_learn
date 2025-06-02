# 字符设备控制LED

## 数据手册

1. 控制的LED原理图
欲达到使用GPIO控制SYSLED的效果,原理图如下
![alt text](image-1.png)
需要控制GPIO1C6引脚

2. 基地址
![alt text](image-2.png)

3. RK3588引脚复用,查询datasheet得到
![alt text](image.png)

4.数据寄存器
![alt text](image-3.png)
![alt text](image-5.png)
![alt text](image-4.png)
![alt text](image-6.png)


## 代码适配部分
