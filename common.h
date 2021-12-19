#ifndef __COMMON_H
#define __COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>

int is_integer(char *arg){
    for (int i = 0; i < strlen(arg); i++)
    {
        if (isdigit(arg[i]) == 0)
              return 0;
    }
    return 1;
}

int _checked(int ret, char* calling_function) {
  if (ret < 0) {
    perror(calling_function);
    exit(EXIT_FAILURE);
  }
  return ret;
}

// The macro allows us to retrieve the name of the calling function
#define checked(call) _checked(call, #call)

/**
 * @brief Send data under the form <size_t len><...data>
 * Function name is 'ssend' instead of 'send' because the latter already exists.
 */
int ssend(int sock, void* data, size_t len) {
  checked(write(sock, &len, sizeof(len)));
  return checked(write(sock, data, len));
}

/**
 * @brief Receive data under the form <size_t len><data...>.
 */
size_t sreceive(int sock, void** dest) {
  size_t nbytes_to_receive;
  if (checked(read(sock, &nbytes_to_receive, sizeof(nbytes_to_receive))) == 0) {
    // Connection closed
    return 0;
  };
  unsigned char* buffer = malloc(nbytes_to_receive);
  if (buffer == NULL) {
    fprintf(stderr, "malloc could not allocate %zd bytes", nbytes_to_receive);
    perror("");
    exit(1);
  }
  size_t total_received = 0;
  while (nbytes_to_receive > 0) {
    size_t received = checked(read(sock, &buffer[total_received], nbytes_to_receive));
    if (received < 0) {
      return total_received;
    }
    total_received += received;
    nbytes_to_receive -= received;
  }
  *dest = buffer;
  return total_received;
}

#endif  // __COMMON_H