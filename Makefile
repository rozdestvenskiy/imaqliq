all:
	gcc server.c -o serverX -w -pthread -std=c99 -D_POSIX_C_SOURCE=199309L
	gcc client.c -o client -w -std=c99
	
