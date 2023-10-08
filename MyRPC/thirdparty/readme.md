# 第三方库/服务-安装和使用
## 1.jsoncpp
### 1.1编译
- jsoncpp-0.5.0使用scons进行编译，在centos下，执行`yum install scons`命令来安装scons。
- 执行`tar -zxf jsoncpp-src-0.5.0.tar.gz`命令来解压源文件。
- 执行`cd jsoncpp-src-0.5.0`命令切换到源文件目录。
- 执行`yum install scons -y`命令安装scons。
- 执行`scons platform=linux-gcc`命令对源文件进行编译。
### 1.2安装
- 执行`mkdir -p /usr/local/jsoncpp/libs`创建目录。
- 在源文件jsoncpp-src-0.5.0目录下执行`cp -r include /usr/local/jsoncpp/`命令，发布头文件。
- 在源文件jsoncpp-src-0.5.0目录下执行`find ./libs -name '*.so' | xargs -I{} cp {} /usr/local/jsoncpp/libs/libjson.so`命令，发布动态链接库。
### 1.3使用
- 在源代码中include jsoncpp相关的头文件。
- 编译的时指定头文件路径，并链接libjson.so库。

## 2.protobuf
### 2.1编译
- 执行`tar -zxf protobuf-cpp-3.6.1.tar.gz`命令来解压源文件。
- 执行`cd protobuf-3.6.1`命令切换到源文件目录。
- 执行`./configure --prefix=/usr/local/protobuf`命令生成编译的makefile。
- 执行`make -j$(nproc)`命令对源文件进行编译。
### 2.2安装
- 执行`make install`发布相关的头文件和链接库。
### 2.3使用
- 在源文件中include protobuf相关的头文件。
- 编译的时指定头文件路径，并链接libprotobuf.so库。

## 3.redis
### 3.1编译
- 执行`tar -zxf redis-stable.tar.gz`命令来解压源文件。
- 执行`cd redis-stable`命令切换到源文件目录。
- 执行`make -j$(nproc)`命令对源文件进行编译。
### 3.2启动
- 编辑当前目录的redis.conf文件，修改daemonize配置为yes，开启requirepass认证配置并设置密码。
- 执行`./src/redis-server ./redis.conf`命令来启动redis服务。

## 4.snappy
### 4.1编译
- 执行`tar -zxf snappy-1.0.5.tar.gz`命令来解压源文件。
- 执行`cd snappy-1.0.5`命令切换到源文件目录。
- 执行`./configure --prefix=/usr/local/snappy`命令生成编译的makefile。
- 执行`make -j$(nproc)`命令对源文件进行编译。
### 4.2安装
- 执行`make install`发布相关的头文件和链接库。
### 4.3使用
- 在源文件中include snappy相关的头文件。
- 编译的时指定头文件路径，并链接libsnappy.so库。