# cmserver

a tiny http web server(epoll+threadpoll).

## issue
+ 对nonbloacking socket写大文件时，会触发EAGAIN错误
+ 客户端关闭连接时，会触发EPIPE信号
+ epoll的ET模式下，应该不断地读，直到EAGAIN为止(accept, read)
+ epoll oneshott的设置


## TODO
+ http header
+ timer