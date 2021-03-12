all:
	gcc server.c -o serverX -w -pthread -std=c99
	gcc client.c -o client -w -std=c99
	
