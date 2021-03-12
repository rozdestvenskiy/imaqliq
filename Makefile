all:
	gcc server.c -o serverX -w -pthread
	gcc client.c -o client -w
	
