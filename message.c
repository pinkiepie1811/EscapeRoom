// Modified from Networking Exercise
#include "message.h"

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/**
 * Send a username and its message across a socket, which both have a header that includes the username and message lengths
 * \param fd    The fd to send a message to
 * \param info  A message_info_t variable that contains the username and the message
 * \return      -1 if the message failed to send, 0 if the send was successful
 */
int send_message(int fd, message_info_t info) {
  // If the username or message is NULL, set errno to EINVAL and return an error
  if ((info.username == NULL) || (info.message == NULL)) {
    errno = EINVAL;
    return -1;
  }

  // -- FOR USERNAME -- //
  // First, send the length of the username in a size_t
  size_t username_len = strlen(info.username) + 1;
  if (write(fd, &username_len, sizeof(size_t)) != sizeof(size_t)) {
    // Writing failed, so return an error
    return -1;
  }

  // Now we can send the username. Loop until the entire username has been written
  size_t bytes_written = 0;
  while (bytes_written < username_len) {
    // Try to write the entire remaining message
    ssize_t rc = write(fd, info.username + bytes_written, username_len - bytes_written);

    // Did the write fail? If so, return an error
    if (rc <= 0) return -1;

    // If there was no error, write returned the number of bytes written
    bytes_written += rc;
  }

  // -- FOR MESSSAGE -- //
  // First, send the length of the message in a size_t
  size_t message_len = strlen(info.message) + 1;
  if (write(fd, &message_len, sizeof(size_t)) != sizeof(size_t)) {
    // Writing failed, so return an error
    return -1;
  }

  // Now we can send the message. Loop until the entire message has been written
  bytes_written = 0;
  while (bytes_written < message_len) {
    // Try to write the entire remaining message
    ssize_t rc = write(fd, info.message + bytes_written, message_len - bytes_written);

    // Did the write fail? If so, return an error
    if (rc <= 0) return -1;

    // If there was no error, write returned the number of bytes written
    bytes_written += rc;
  }

  return 0;
} // send_message

/**
 * Receive a message from a socket and return the message string (which must be freed later)
 *
 * \param fd  The fd to receive the message from
 * \return    A message_info_t variable containing the username of the sender and the received message.
 *                             Returns a message_info_t with one of the fields as NULL upon error.
 */
message_info_t receive_message(int fd) {
  // Variable to hold the values that we will return (the username and the message)
  message_info_t info;

  // -- FOR USERNAME -- //
  // First, try to read in the username length
  size_t username_len;
  if (read(fd, &username_len, sizeof(size_t)) != sizeof(size_t)) {
    // Reading failed. Set username to NULL (indicates an error) and return
    info.username = NULL;
    return info;
  }

  // Now, make sure the username length is reasonable
  if (username_len > MAX_MESSAGE_LENGTH) {
    errno = EINVAL;
    info.username = NULL;
    return info;
  }

  // Allocate space for the username
  char* username_result = malloc(username_len);

  // Try to read the username. Loop until the entire username has been read
  size_t bytes_read = 0;
  while (bytes_read < username_len) {
    // Try to read the entire remaining message
    ssize_t rc = read(fd, username_result + bytes_read, username_len - bytes_read);

    // Did the read fail? If so, set the username to NULL and return
    if (rc <= 0) {
      free(username_result);
      info.username = NULL;
      return info;
    }

    // Update the number of bytes read
    bytes_read += rc;
  }
  // Upon successful read, store the result in our variable's field for the username
  info.username = username_result;

  // -- FOR MESSAGE -- //
  // First, try to read in the message length
  size_t message_len;
  if (read(fd, &message_len, sizeof(size_t)) != sizeof(size_t)) {
    // Reading failed. Set message to NULL (indicates an error) and return
    info.message = NULL;
    return info;
  }

  // Now, make sure the message length is reasonable
  if (message_len > MAX_MESSAGE_LENGTH) {
    errno = EINVAL;
    info.message = NULL;
    return info;
  }

  // Allocate space for the message
  char* message_result = malloc(message_len);

  // Try to read the message. Loop until the entire message has been read
  bytes_read = 0;
  while (bytes_read < message_len) {
    // Try to read the entire remaining message
    ssize_t rc = read(fd, message_result + bytes_read, message_len - bytes_read);

    // Did the read fail? If so, set message to NULL and return
    if (rc <= 0) {
      free(message_result);
      info.message = NULL;
      return info;
    }

    // Update the number of bytes read
    bytes_read += rc;
  }
  // Upon successful read, store the result in our variable's field for the username
  info.message = message_result;

  // Return our variable with its fields filled from the reads
  return info;
} // receive_message