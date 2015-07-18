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
    
    /** Command line check **/
    if (argc > 2 ) on_error("Uso: tcp_echo [porta]\n");
    if (argc == 2)  port = atoi(argv[1]);/** Number conversion **/
    
    /** Winsock initialization **/
    err = WSAStartup(MAKEWORD(2,2), &wsadata);
    if (err != 0)
        on_error("Error in WSAStartup", &err);

    /** Socket initialization **/
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == INVALID_SOCKET)
        on_error("Could not create socket");

    /** Socket config **/
    memset(&server, 0, sizeof(server)); /** Zeroing **/
    server.sin_family = AF_INET;        /** Family **/
    server.sin_port = htons(port);      /** Number conversion **/
    server.sin_addr.s_addr = INADDR_ANY;/** Accept from any IP **/ 

    /** bind & listen **/
    const BOOL opt_val = TRUE;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt_val, sizeof(opt_val));
    err = bind(server_fd, (struct sockaddr *) &server, sizeof(server));
    if (err == SOCKET_ERROR)
        on_error("Could not bind socket");
    err = listen(server_fd, 1);
    if (err == SOCKET_ERROR)
        on_error("Could not listen on socket");

    printf("Server listening on port %d\n", port);

    while (1)
    {
        int client_len = sizeof(client);
        client_fd = accept(server_fd, (struct sockaddr *) &client, &client_len);

        if (client_fd == INVALID_SOCKET)
            on_error("Could not establish a new connection");

        do
        {
        	/** Read data (read = bytes received) **/
            int read = recv(client_fd, buf, BUFFER_SIZE, 0);

            if (read == 0)
                break; /** End of connection **/

            if (read == SOCKET_ERROR)
            {
                err = WSAGetLastError();
                if ((err != WSAENOTCONN) && (err != WSAECONNABORTED) && (err == WSAECONNRESET))
                    on_error("Error reading data", &err);
                break;
            }

			/** Transmit back received data **/
                int sent = send(client_fd, buf, read, 0);
                if (sent == SOCKET_ERROR)
                {
                    err = WSAGetLastError();
                    if ((err != WSAENOTCONN) && (err != WSAECONNABORTED) && (err == WSAECONNRESET))
                        on_error("Error writing to client", &err);
                }
			
        }
        while (true);

        closesocket(client_fd);
    }

    WSACleanup();
    return 0;
}
