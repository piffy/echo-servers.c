/**Copyright (c) 2015 Mathias Buus, Marcello Missiroli & Remy Lebeau */
#include <stdio.h>
#include <stdlib.h>
#include <winsock.h>

#define BUFFER_SIZE 1024

void on_error(char *s, int *errCode = NULL)
{
    int err = (errCode) ? *errCode : WSAGetLastError();
    fprintf(stderr, "%s: %d\n", s, err);
    fflush(stderr);
    WSACleanup();
    exit(1);
}

int main(int argc, char *argv[])
{
    WSADATA wsadata; 
    SOCKET server_fd, client_fd;
    struct sockaddr_in server, client;
    int port = 6666, err; 
    char buf[BUFFER_SIZE];
    
    /** Controllo linea di comando **/
    if (argc > 2 ) on_error("Uso: tcp_echo [porta]\n");
    if (argc == 2)  port = atoi(argv[1]);/** Conversione numero **/
    
    /** Inizializzazione Winsock **/
    err = WSAStartup(MAKEWORD(2,2), &wsadata);
    if (err != 0)
        on_error("Errore in WSAStartup", &err);

    /** Inizializzazione socket **/
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == INVALID_SOCKET)
        on_error("Non ho potuto creare il socket");

    /** Configurazione socket **/
    memset(&server, 0, sizeof(server)); /** Azzeramento **/
    server.sin_family = AF_INET;        /** Famiglia **/
    server.sin_port = htons(port);      /** Conversione numero **/
    server.sin_addr.s_addr = INADDR_ANY;/** Accetta qualsiasi IP **/ 

    /** bind & listen **/
    const BOOL opt_val = TRUE;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt_val, sizeof(opt_val));
    err = bind(server_fd, (struct sockaddr *) &server, sizeof(server));
    if (err == SOCKET_ERROR)
        on_error("Non ho potuto fare il bind del socket");
    err = listen(server_fd, 1);
    if (err == SOCKET_ERROR)
        on_error("Non ho potuto mettermi in ascolto sul socket");

    printf("Server in ascolto sulla porta %d\n", port);

    while (1)
    {
        int client_len = sizeof(client);
        client_fd = accept(server_fd, (struct sockaddr *) &client, &client_len);

        if (client_fd == INVALID_SOCKET)
            on_error("Non riesco a stabilire una nuova connessione");

        bool continua= true;
        do
        {
        	/** Leggi dati (read = numero di byte ricevuti) **/
            int read = recv(client_fd, buf, BUFFER_SIZE, 0);

            if (read == 0)
                break; /** Fine connessione **/

            if (read == SOCKET_ERROR)
            {
                err = WSAGetLastError();
                if ((err != WSAENOTCONN) && (err != WSAECONNABORTED) && (err == WSAECONNRESET))
                    on_error("Errore nella lettura dal client", &err);
                break;
            }

			/** Trasmetti i dati ricevuti **/
                int sent = send(client_fd, buf, read, 0);
                if (sent == SOCKET_ERROR)
                {
                    err = WSAGetLastError();
                    if ((err != WSAENOTCONN) && (err != WSAECONNABORTED) && (err == WSAECONNRESET))
                        on_error("Errore nella scrittura verso il client", &err);
                }
			
        }
        while (continua);

        closesocket(client_fd);
    }

    WSACleanup();
    return 0;
}
