# MUA远控木马

因为时间问题，写的还简陋，而且未经大量测试，程序的健壮性还不行，劳请师傅们多多包涵。

仅供作者本人学习之用，请勿用于非法用途。

水平有限，请您批评指正。

具体可戳：[木马编程入门 - 『编程语言区』 - 吾爱破解](https://www.52pojie.cn/thread-1382127-1-1.html)

## 写在前面

### 2022.11.12

本项目开发之时由于太赶工期，架构上比较混乱，目前我在重构新版，功能上已经超过原版，但我想尽可能多得完善其他新功能，所以新版的开源还要过些时日，敬请期待:-)

## 关于安装

Debug编译的话，只生成MuaClient.exe，用于测试。命令行下`MuaClient.exe Host Port`即可运行。

Release编译的话，生成MuaClient.dll，InstallMuaClient.exe，SystemService.exe.

InstallMuaClient.exe用于安装MuaClient被控端。安装时三个文件需在同一文件夹内。

## 关于测试

我是在win10上开发的，环境是vs2017。

请务必在虚拟机内测试。

MuaClient是32位的，使用InstallMuaClient.exe安装后即可。

MuaServer源码中没有限定64位或32位，不过开发和测试都是以32位为主，64位具体能不能用我没测试。运行时需要将相应的HPSocket_??.dll放在程序所在目录，该dll可在`.\MuaServer\HPSocket`中找到。
