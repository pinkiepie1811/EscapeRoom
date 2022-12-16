// Modified from Peer-to-Peer Chat Lab
#pragma once

#define MAX_MESSAGE_LENGTH 2048

/**
 * Struct with username of the message and the message.
 * username: The username of the user that sent the message.
 * message: The message that the user sent.
 */
typedef struct {
    char* username;
    char* message;
} message_info_t;

/**
 * Send a username and its message across a socket, which both have a header that includes the username and message lengths.
 * \param fd    The fd to send a message to.
 * \param info  A message_info_t variable that contains the username and the message.
 * \return      -1 if the message failed to send, 0 if the send was successful.
 */
int send_message(int fd, message_info_t message);

/**
 * Receive a message from a socket and return the message string (which must be freed later).
 *
 * \param fd  The fd to receive the message from.
 * \return    A message_info_t variable containing the username of the sender and the received message.
 *                             Returns a message_info_t with one of the fields as NULL upon error.
 */
message_info_t receive_message(int fd);