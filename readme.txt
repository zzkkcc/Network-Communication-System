Basic Information:
	Kaichen Zhang	USCID:2041-8336-42


How to run project:

In this code, all process(phase 1,2&3)are completed described as
the requirement. To open and test the code you need to follow steps
shown as below:

1.using command window, enter into socket document. as ..../ee450_kaichenz_session1:
2.input "make all" to compile all c-files and a header-file
3.input "make server_or" to run server_or server
4.input "make server_and" to run server_and server
5.input "make edge" to run edge server
6.input "./client job.txt" starts the client with job file

After these step, you will see expected results of the four server/client.


Introductions to files & codes

In the socket document, four required c-files(edge, server_or, server_and,
client) and a txt file(job.txt) can been seen as the requirement. Among them,
client is used for reading txt file and transimit to the edge, while edge is 
used for making descisions of which back-end server, while server_and & server_or
compute &,| operation seperately.

What is more,  udpfun.c(together with udpfun.h) is utilized in three c files(
edge,server_or & server_and file), for example, edge file is binded with udpfun
file in order to operate(same rules as server_or, server_and file). udpfun.c 
includes three udp funtions. With this udpfun.h, codes look more briefly.

Also, in the code, messages are exchanged all in 'char' format, while-loop is 
NOT used for servers and client. 

txt files with wrong format(such as no 'or','and', empty file, etc) are not 
considered, it may cause idiosyncrasy.

Some parts of the codes are used from the Beej's tutorial.(They are commented 
in the codes).
