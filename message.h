// Modified from Networking Exercise
#pragma once

#define MAX_MESSAGE_LENGTH 2048

/**
 * Struct with username of the message and the message that was sent in the chat
 * username: The username of the user that sent the message
 * message: The message that the user sent
 */
typedef struct {
    char* username;
    char* message;
} message_info_t;

// Send a username and message across a socket with a header that includes the
// username and message length. Returns non-zero value if an error occurs.
int send_message(int fd, message_info_t message);

// Receive a username and message from a socket and return the message_info_t struct
// (the fields must be freed later). Sets the username and message strings to NULL when an error occurs
// (and returns right away). The username and message strings must be checked to see if they are
// NULl before proceeding (this is how we error check).
message_info_t receive_message(int fd);