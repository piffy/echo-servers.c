/**Copyright (c) 2014 Mathias Buus & Marcello Missiroli */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define BUFFER_SIZE 1024
void on_error(char *s) { fprintf(stderr,"%s\n",s); fflush(stderr); exit(1); }

int main (int argc, char *argv[]) {
  /** Controllo linea di comando **/
  if (argc != 2) on_error("Uso: tcp_echo [porta]\n");

  int port = atoi(argv[1]);/** Conversione numero **/

  int server_fd, client_fd, err;
  struct sockaddr_in server, client;
  char buf[BUFFER_SIZE];

  /** Creazione socket  **/
  server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) on_error("Non ho potuto creare il socket\n");
  /** Impostazioni del socket del server */
  server.sin_family = AF_INET;                /** Famiglia internet **/
  server.sin_port = htons(port);              /** Conversione numero **/
  server.sin_addr.s_addr = htonl(INADDR_ANY); /** Accetta da qualsiasi IP **/

  int opt_val = 1;
  setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof opt_val);
  /** bind & listen **/
  err = bind(server_fd, (struct sockaddr *) &server, sizeof(server));
  if (err < 0) on_error("Non ho potuto fare il bind del socket\n");
  err = listen(server_fd, 128);
  if (err < 0) on_error("Non ho potuto mettermi in ascolto sul socket\n");
  printf("Il server e' in ascolto sulla porta %d\n", port);

  while (1) {
    socklen_t client_len = sizeof(client);
    client_fd = accept(server_fd, (struct sockaddr *) &client, &client_len);
    /** Comunicazione accettata, ora abbiamo il socket del client **/

    if (client_fd < 0) on_error("Non riesco a stabilire una nuova connessione\n");

    while (1) { /** Continua all'infinito **/
      int read = recv(client_fd, buf, BUFFER_SIZE, 0);

      if (!read) break; /** Fine lettura **/
      if (read < 0) on_error("Errore nella lettura dal client\n");

      err = send(client_fd, buf, read, 0);
      if (err < 0) on_error("Errore nella scrittura verso il client\n");
    }
  }

  return 0;
}
