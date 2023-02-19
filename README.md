# a simple udp socket service #

dev env: debian 9 and ubuntu 22.04(via WSL) with their latest gcc from apt-get

## build and run server ##
```
$ gcc server.c -o server.o
```

usage: server &lt;port&gt;
```
$ server.o 65432
``` 

## build and run client ##
may need -lm option to gcc compiler for libm.a
```
$ gcc client.c -o client.o -lm
```

usage: client &lt;ip&gt; &lt;port&gt;
```
$ client.o 10.0.0.10 65432 
```

socket retry mechanism uses exponential backoff alogrithm with 8 seconds max interval

refer to source code comments
