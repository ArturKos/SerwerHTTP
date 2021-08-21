#include "httpd.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <netinet/in.h>
//#include <process.h> 
#define BUFFLEN 512
bool ok = false;
int main(int argc, char* argv[])
{
	int sserver;
	int sklient;
	int sock_len;
	char error404[] = "Error 404 Nie odnaleziono pliku na serwerze!";
	char server_ok_html[] = "HTTP/1.1 200 OK\r\nServer: Artur Kos Projekt\r\nContent-Type: text/html; charset=utf-8\r\nContent-Length: ";
	char server_ok_css[] = "HTTP/1.1 200 OK\r\nServer: Artur Kos Projekt\r\nContent-Type: text/css; charset=utf-8\r\nContent-Length: ";
	char server_ok_bin_png[] = "HTTP/1.1 200 OK\r\nServer: Artur Kos Projekt\r\nAccept-Ranges: bytes\r\nContent-Type: image/png\r\nContent-Length: ";
	char server_ok_bin_gif[] = "HTTP/1.1 200 OK\r\nServer: Artur Kos Projekt\r\nContent-Type: image/gif\r\nContent-Length: ";
	char server_ok_bin_jpg[] = "HTTP/1.1 200 OK\r\nServer: Artur Kos Projekt\r\nContent-Type: image/jpg\r\nContent-Length: ";
	char server_err[] = "HTTP/1.1 404 Nie znaleziono\r\nServer: Artur Kos Projekt\r\nContent-Type: text/html; charset=utf-8\r\nContent-Length: ";
	struct sockaddr_in klient;
	pid_t PID;
	time_t czas;
	char url[256];
	char bufor[BUFFLEN + 1];
	int len, flen;
	char buf_czas[128];
	char fbuf[BUFFLEN + 1];
	FILE* doc;
	char dir[512];
	char bdir[1024];
	char log[2048];
	char ext[15];
	if (argc != 3)
	{
		printf("%s \n", "Program wymaga podania 2 argumentow.");
		printf("%s \n", "myhttpd port dir");
		return 1;
	}
	printf("%s \n", "Inicjuje server.");
	if ((sserver = init(argv[1])) < 0)
	{
		printf("%s \n", "Wystapily problemy z utworzeniem gniazda.");
		return 1;
	}
	printf("%s%s\n", "Server oczekuje na polaczenia na porcie: ", argv[1]);
	printf("%s %d\n", "PID procesu macierzystego: ", getpid());
	/* główna pętla serwera */
	sock_len = sizeof(struct sockaddr_in);
	for (;;)
	{

		if ((sklient = accept(sserver, (struct sockaddr*)&klient, &sock_len)) < 0)
		{
			printf("%s \n", "Wystapily problemy z obsluga klienta.\n");
			return 1;
		}
		/* tworzę nowy proces servera */

		if ((PID = fork()) == -1)
		{
			printf("%s \n", "Wystapily problemy z utworzeniem nowego procesu.\n");
			close(sklient);
			continue;
		}
		else
			if (PID > 0) //czy jest to proces macierzysty?
			{
				close(sklient);
				continue;
			}
		/* proces potomny będzie obsługiwał połączenie z klientem */
		time(&czas);
		strftime(buf_czas, sizeof buf_czas, "[DATA: %d.%m.%Y] [GODZINA: %H:%M:%S]", localtime(&czas));
		// printf("%s %s\n","Witaj kliencie: ", inet_ntoa(klient.sin_addr));
		// printf("%s %d\n","PID procesu: ", getpid());
		// printf("Czas: %s\n", buf_czas);
		bzero(log, sizeof log);
		bzero(url, sizeof url);
		bzero(bufor, sizeof bufor);

		if ((len = recv(sklient, bufor, sizeof bufor, 0)) > 0)
		{
			//bufor[len] = '\0';
			//printf("%s\n", bufor);
			GetDir(bufor, dir);
			bzero(bdir, sizeof bdir);
			strcat(bdir, argv[2]);
			strcat(bdir, dir);

			if (strlen(dir) == 1) strcat(bdir, "index.html");
			ok = false;
			doc = fopen(bdir, "rb");
			if (doc != NULL)
			{
				ok = true;
				bzero(fbuf, sizeof fbuf);
				GetExt(bdir, ext);
				printf("%s %d\n", "PID procesu potomka: ", getpid());
				if (strncmp(ext, "html", 4) == 0)
				{
					char tmp[200];
					sprintf(tmp, "%s%lu\r\n\r\n", server_ok_html, fsize(doc));
					write(sklient, tmp, strlen(tmp));  //wysyłam nagłówek wiadomośći, ok 
				}
				else
					if (strcmp(ext, "jpg") == 0)
					{
						char tmp[200];
						sprintf(tmp, "%s%lu\r\n\r\n", server_ok_bin_jpg, fsize(doc));
						write(sklient, tmp, strlen(tmp));  //wysyłam nagłówek wiadomośći, ok 
					}
					else
						if (strncmp(ext, "png", 3) == 0)
						{
							char tmp[500];
							sprintf(tmp, "%s%lu\r\n\r\n", server_ok_bin_png, fsize(doc));
							write(sklient, tmp, strlen(tmp));  //wysyłam nagłówek wiadomośći, ok 
						}
						else
							if (strcmp(ext, "gif") == 0)
							{
								char tmp[500];
								sprintf(tmp, "%s%lu\r\n\r\n", server_ok_bin_gif, fsize(doc));
								write(sklient, tmp, strlen(tmp));  //wysyłam nagłówek wiadomośći, ok 
							}
							else
								if (strcmp(ext, "css") == 0)
								{
									char tmp[500];
									sprintf(tmp, "%s%lu\r\n\r\n", server_ok_css, fsize(doc));
									write(sklient, tmp, strlen(tmp));  //wysyłam nagłówek wiadomośći, ok 
								}
				while ((flen = fread(fbuf, sizeof(char), sizeof fbuf, doc)) > 0)
					send(sklient, fbuf, flen, 0); //write(sklient , fbuf , flen);   
				fclose(doc);
			}
			else
			{
				char tmp[500];
				sprintf(tmp, "%s\r\n\r\n%s\r\n", server_err, error404);
				write(sklient, tmp, strlen(tmp));  //wysyłam nagłówek wiadomośći, ERROR blad otwarcia pliku
			}
			if (ok)
				sprintf(log, "%s IP_HOSTA:[%s] PID_PROCESU: [%d] URL_ZADANY: [%s] STATUS_ODPOWIEDZI: [OK]\n", buf_czas, inet_ntoa(klient.sin_addr), getpid(), bdir); else //dane do logu
				sprintf(log, "%s IP_HOSTA:[%s] PID_PROCESU: [%d] URL_ZADANY: [%s] STATUS_ODPOWIEDZI: [NIE ZNALEZIONO PLIKU]\n", buf_czas, inet_ntoa(klient.sin_addr), getpid(), bdir);
			push_log(log); //zapisuje log
		}
		//printf("%s\n", "Zakonczylem prace z klientem.");
		close(sklient); //zamykam połączenie z klientem
	}

	finito(sserver); // zamknięcie gniazda tutaj nigdy nie dochodzi program :)

	return 0;
}