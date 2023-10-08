# 1.概述
这是《Linux后端开发工程实践》的代码仓库。
# 2.目录结构
``` 
BackEnd
├── Chapter03
├── Chapter04
├── Chapter05
├── Chapter07
├── Chapter08
├── Chapter09
├── Chapter10
├── Chapter12
├── LICENSE
├── MyRPC
└── readme.md
```
Chapter03到Chapter10目录分别保存着第3章到第10章的代码，Chapter12目录只保存了一些独立的示例代码，
第11章到第14章的其他代码都保存在MyRPC目录中， 因为第11章到第14章的代码都是用来实现MyRPC框架的。
## 2.1 MyRPC目录结构
这里特别说明一下MyRPC这个子目录的目录结构
```
MyRPC
├── common
├── core
├── protocol
├── service
├── test
├── thirdparty
└── tool 
```
common子目录为第11章公共代码集合的代码，protocol子目录为第12章应用层协议设计与实现的代码，
core子目录和tool子目录为第13章MyRPC框架设计与实现的代码，service子目录为第14章简单微服务集群的代码，
test子目录为所有的单元测试代码，thirdparty子目录为依赖的第三方代码或者服务。