## 简介
**该服务器是利用C++11完成的多线程web服务器,可以处理静态资源请求,支持Http连接的超时管理

##开发环境
- **操作系统**: CentOS 7.3 64位
- **代码工具**: vim+git
- **编译器**  : gcc 9.1.1 
- **程序构建**: make
- **调试工具**: gdb
- **压测工具**: webbench

##技术应用
- 并发模型:Reactor
- 利用Epoll实现IO多路复用,触发方式为边沿触发+EPOLLONESHOT,数据非阻塞IO
- 为了避免线程创建和销毁的开销,采用线程池
- 由主线程监听套接字,子线程处理套接字上发生的事件
- 事件的添加和处理使用生产者-消费者模式,维护一个任务队列,并用互斥锁和条件锁保证操作的高效互斥与同步
- 实现基于小根堆的定时器,用于管理Http连接的超时断开
- 为了配个非阻塞IO,在应用层定义了能够循环读写的Buffer类,每个Http连接都有输入和输出缓冲区
- 将socket操作封装,将ip和port封装,使函数更容易被正确调用

## 文档

