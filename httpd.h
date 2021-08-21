#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <sys/select.h>
#include <unistd.h>
#define MAXRCVLEN 1000
#define PORTNUM 110
#define fname_log "myhttpd.log"

void push_log(const char log[])
{
	FILE* l;
	l = fopen(fname_log, "a+");

	if (l != NULL)
	{
		fwrite(log, sizeof(char), strlen(log), l);
		fclose(l);
	}
}
unsigned long fsize(FILE* f)
{
	fseek(f, 0, SEEK_END);
	unsigned long len = (unsigned long)ftell(f);
	fseek(f, 0, SEEK_SET);
	return len;
}
int init(const char* port)
{

	int  server_socket;
	struct sockaddr_in server;

	if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) return -1;
	memset(&server, 0, sizeof server);
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(atoi(port));
	/* powiązanie gniazda serwera z adresem */
	if (bind(server_socket, (struct sockaddr*)&server, sizeof(server)) < 0) return -1;
	/* ustawiam gniazdo server gniazdem nasłuchującym */
	if (listen(server_socket, 10) < 0) return -1;

	return server_socket;

}
void GetDir(const char d[], char dir[])
{
	int i, idx = 0;
	bzero(dir, sizeof dir);
	for (i = 4; i < strlen(d); i++)
		if ((d[i] != ' ') && (d[i] != '\r') && (d[i] != '\n'))
			dir[idx++] = d[i]; else break;
	dir[idx] = '\0';
}
void GetExt(const char path[], char ext[])
{
	int p, i, idx = 0;
	bzero(ext, sizeof ext);
	for (p = strlen(path); p > 0; p--)
		if (path[p] == '.') break;

	for (i = p + 1; i < strlen(path); i++)
		if ((path[i] != ' ') && (path[i] != '\r') && (path[i] != '\n'))
			ext[idx++] = path[i]; else break;
	ext[idx] = '\0';
}
bool logowanie(int socket, const char* login, const char* haslo)
{
	char buffer[MAXRCVLEN + 1];
	bzero(buffer, MAXRCVLEN + 1);
	int len;
	char komenda[] = "USER ";
	char pass[] = "PASS ";
	strcat(komenda, login);
	strcat(komenda, "\r\n");
	send(socket, komenda, strlen(komenda), 0);
	//printf("%s\n",komenda);
	len = recv(socket, buffer, MAXRCVLEN, 0);
	strcat(buffer, "\r\n");
	buffer[len] = '\0';
	//printf("login %s\n",buffer);
	if (strncmp(buffer, "+OK", 3) != 0)
		return false;
	bzero(buffer, MAXRCVLEN + 1);
	strcpy(komenda, pass);
	strcat(komenda, haslo);
	strcat(komenda, "\r\n");
	send(socket, komenda, strlen(komenda), 0);
	// printf("%s\n",komenda);
	len = recv(socket, buffer, MAXRCVLEN, 0);
	strcat(buffer, "\r\n");
	//  buffer[len]='\0';
	 //printf("haslo %s\n",buffer);
	if (strncmp(buffer, "+OK", 3) != 0)
		return false;

	return true;
}
void wyloguj(int socket)
{
	char komenda[] = "QUIT\r\n";
	int len;
	char buffer[MAXRCVLEN + 1];
	send(socket, komenda, strlen(komenda), 0);
	len = recv(socket, buffer, MAXRCVLEN, 0);

	if (strncmp(buffer, "+OK", 3) != 0)
		printf("Wystąpiły problemy z poprawnym wylogowaniem.\n");//else
	 //   printf("Wylogowano poprawnie.\n");  
}

void finito(int socket)
{
	close(socket);
}
