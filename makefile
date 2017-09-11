edge_obj=	edge.o	udpfun.o
server_or_obj=	server_or.o	udpfun.o
server_and_obj=	server_and.o	udpfun.o

all:	$(edge_obj)	client.o	$(server_or_obj)	$(server_and_obj)
	cc	-o	EDGE	$(edge_obj)
	cc	-o	client		client.o
	cc	-o	SERV_OR	$(server_or_obj)
	cc	-o	SERV_AND	$(server_and_obj)

edge.o:	edge.c	udpfun.h
	cc	-c	edge.c
udpfun.o:udpfun.c	udpfun.h
	cc	-c	udpfun.c
server_or.o:server_or.c	udpfun.h
	cc	-c	server_or.c
server_and.o:server_and.c	udpfun.h
	cc	-c	server_and.c
client.o:client.c
	cc	-c	client.c
edge:	
	./EDGE
server_or:
	./SERV_OR
server_and:
	./SERV_AND
.PHONY:	clean
clean:
	rm	EDGE	edge.o	client	client.o	SERV_OR	\
		server_or.o	SERV_AND	$(server_and_obj)	
