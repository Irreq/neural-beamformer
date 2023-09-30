#ifndef _RECEIVER_H_
#define _RECEIVER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "config.h"
#include <stdbool.h>
#include "ring_buffer.h"


/// @brief FPGA Protocol Version 2
typedef struct _msg
{
    // int8_t b;
    // int8_t c;
    // int16_t d;
    // float a[N];
    u_int16_t frequency;
    int8_t n_arrays;
    int8_t protocol_ver;
    int32_t counter;
    
    int32_t stream[N_SENSORS];
} msg;

// typedef struct _msg
// {
//     int8_t b;
//     float a[8];
// } msg;

int init_receiver();
int receive(ring_buffer *rb);
void stop_receiving();



msg *create_msg();
msg *destroy_msg(msg *msg);



/// @brief Creates and binds the socket to a server ip and port.
/// @pre Requires the SERVER_IP and UDP_PORT to be correctly specified in the header file.
/// @return A socket descriptor if successfully created and bound. -1 if an error occured.
int create_and_bind_socket();

/// @brief Closes the socket descriptor.
/// @param socket_desc A socket file descriptor.
/// @return -1 if error occured.
int close_socket(int socket_desc);

/// @brief Receives the first message and returns the number of arrays
/// @param socket_desc
/// @return The number of connected arrays
int receive_header_data(int socket_desc);

void receive_to_buffer(int socket_desc, float *out, msg *message, int n_arrays);

#ifdef __cplusplus
}
#endif

#endif