# MUA远控木马

因为时间问题，写的还简陋，而且未经大量测试，程序的健壮性还不行，劳请师傅们多多包涵。

仅供作者本人学习之用，请勿用于非法用途。

水平有限，请您批评指正。

具体可戳：[木马编程入门 - 『编程语言区』 - 吾爱破解](https://www.52pojie.cn/thread-1382127-1-1.html)

## 写在前面

本项目开发之时由于太赶工期，架构上比较混乱，目前我在重构新版，功能上已经超过原版。~~但我想尽可能多得完善其他新功能，所以新版的开源还要过些时日，敬请期待:-)~~

[点击跳转至 MUA远控木马 V2版本](https://github.com/iyzyi/MUA-Remote-Access-Trojan-V2)

## 关于安装

`Debug`编译时，只生成`MuaClient.exe`，用于测试。命令行下`MuaClient.exe Host Port`即可运行。

`Release`编译时，生成`MuaClient.dll`, `InstallMuaClient.exe`, `SystemService.exe`。其中的`InstallMuaClient.exe`用于安装`MuaClient`被控端。安装时三个文件需在同一文件夹内。

## 关于测试

我是在`win10`上开发的，环境是`vs2017`。

请务必在虚拟机内测试。

`MuaClient`是32位的，使用`InstallMuaClient.exe`安装即可。

`MuaServer`源码中没有限定64位或32位，不过开发和测试都是以32位为主，64位具体能不能用我没测试。运行时需要将相应的`HPSocket_??.dll`放在程序所在目录，该`dll`可在`.\MuaServer\HPSocket`中找到。
