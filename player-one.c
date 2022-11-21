#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "socket.h"

void* player_two_connection(void* client_socket_fd) {


  return NULL;
}

int main() {

  // Open a server socket
  unsigned short port = 0;
  int server_socket_fd = server_socket_open(&port);
  if (server_socket_fd == -1) {
    perror("Server socket was not opened");
    exit(EXIT_FAILURE);
  }

  // Start listening for connections, with a maximum of one queued connection
  if (listen(server_socket_fd, 1)) {
    perror("listen failed");
    exit(EXIT_FAILURE);
  }

  // Display title screen with port number on it
  // printf("Server listening on port %u\n", port);

  // Accept connection

  // Wait for a client to connect
  int client_socket_fd = server_socket_accept(server_socket_fd);
  if (client_socket_fd == -1) {
    perror("accept failed");
    exit(EXIT_FAILURE);
  }

  printf("Client connected!\n");

  pthread_t thread;

  if (pthread_create(&thread, NULL, player_two_connection, (void*)&client_socket_fd) != 0) {
    perror("pthread_create failed");
    exit(EXIT_FAILURE);
  }



  

  close(server_socket_fd);
}