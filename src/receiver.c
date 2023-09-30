#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>
#include <unistd.h>
#include <stdbool.h>
#include "receiver.h"
#include "utils.h"
#include "ring_buffer.h"

// #define UDP_ADDRESS "127.0.0.1"
// #define PORT 12345

#define FPGA_PROTOCOL_VERSION 1

msg *message;
int socket_desc;
ring_buffer *rb;


msg *create_msg()
{
    return (msg *)calloc(1, sizeof(msg));
}

msg *destroy_msg(msg *message)
{
    free(message);
    message = NULL;
    return message;
}


/**
 * @brief Capture the header data for the start of the program
 * 
 * @param socket_desc 
 * @return int 
 */
int receive_header_data(int socket_desc)
{
    msg *message = (msg *)calloc(1, sizeof(msg));
    if (recv(socket_desc, message, sizeof(msg), 0) < 0)
    {
        printf("Couldn't receive\n");
        return -1;
    }

    printf("Received\n");
    // int8_t n_arrays = message->n_arrays;
    // if (message->protocol_ver != FPGA_PROTOCOL_VERSION)
    // {
    //     return -1;
    // }
    free(message);

    int n_arrays = 2;
    return n_arrays;
}

int close_socket(int socket_desc)
{
    return close(socket_desc);
}

int create_and_bind_socket()
{
    int socket_desc;
    struct sockaddr_in server_addr;

    socket_desc = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (socket_desc < 0)
    {
        printf("Error creating socket\n");
        return -1;
    }
    printf("Socket created successfully\n");

    // Set port and IP:
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(UDP_PORT);
    server_addr.sin_addr.s_addr = inet_addr(UDP_ADDRESS);

    // Bind to the set port and IP:
    if (bind(socket_desc, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        printf("Couldn't bind socket to the port\n");
        return -1;
    }
    printf("Binding complete\n");

    return socket_desc;
}


#include <stdint.h>

void stop_receiving() {
    close_socket(socket_desc);
    destroy_msg(message);
    destroy_ring_buffer(rb);
}

int receive(ring_buffer *rb) {
    if (recv(socket_desc, message, sizeof(msg), 0) < 0)
    {
        printf("Couldn't receive\n");
        return -1;
    }

    write_buffer_single_int32(rb, &message->stream[0]);

    return 1;
}

int init_receiver()
{
    // Create UDP socket:
    socket_desc = create_and_bind_socket();
    if (socket_desc == -1)
    {
        exit(1);
    }
    message = create_msg();

    int n_arrays = receive_header_data(socket_desc);
    if (n_arrays == -1)
    {
        exit(1);
    }

    return 1;
}




int _main() {

    // Create UDP socket:
    socket_desc = create_and_bind_socket();
    if (socket_desc == -1)
    {
        exit(1);
    }
    message = create_msg();

    rb = create_ring_buffer();

    signal(SIGINT, stop_receiving);
    signal(SIGKILL, stop_receiving);

    int n_arrays = receive_header_data(socket_desc);
    if (n_arrays == -1)
    {
        exit(1);
    }

    printf("\n");

    while (1) {
        if (recv(socket_desc, message, sizeof(msg), 0) < 0)
        {
            printf("Couldn't receive\n");
            return -1;
        }


        // printf("%d %d %d %d: ", message->frequency, message->n_arrays, message->protocol_ver, message->counter);


        // for (int i = 0; i < N_SENSORS; i++) {
        //     float val = normalize_int32(message->stream[i]);
        //     printf("%f ", val);
        // }

        write_buffer_single_int32(rb, &message->stream[0]);

        float buf[N_SENSORS][BUFFER_LENGTH];

        read_buffer_all(rb, &buf[0]);

        // printf("\r                                                                              \r");
        printf("\n");
        int n = 10;
        for (int i = 0; i < n; i++)
        {
            printf("%f ", buf[0][BUFFER_LENGTH - n + i]);
        }
        // printf("\r");

        // printf("\r                                                             ");


    }
}

// int _main() {
//     int sockfd;
//     struct sockaddr_in server_addr;
//     struct sockaddr_in client_addr;
//     socklen_t client_addr_len = sizeof(client_addr);
//     char buffer[16];  // Assuming 4 floats (4 * 4 bytes) will be received

//     // Create a UDP socket
//     if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
//         perror("Socket creation failed");
//         exit(1);
//     }

//     memset(&server_addr, 0, sizeof(server_addr));
//     memset(&client_addr, 0, sizeof(client_addr));

//     // Configure server address
//     server_addr.sin_family = AF_INET;
//     server_addr.sin_addr.s_addr = INADDR_ANY;
//     server_addr.sin_port = htons(PORT);

//     // Bind the socket to the server address
//     if (bind(sockfd, (const struct sockaddr*)&server_addr, sizeof(server_addr) < 0)) {
//         perror("Binding failed");
//         exit(1);
//     }

//     while (1) {
//         int n = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr*)&client_addr, &client_addr_len);

//         if (n < 0) {
//             perror("Error in recvfrom");
//             exit(1);
//         }

//         // Unpack the received binary data into an array of floats
//         float received_array[4];
//         if (n == sizeof(received_array) * sizeof(float)) {
//             memcpy(received_array, buffer, sizeof(received_array));
//             printf("Received Array: %.2f %.2f %.2f %.2f\n", received_array[0], received_array[1], received_array[2], received_array[3]);
//         } else {
//             printf("Received data size is incorrect.\n");
//         }
//     }

//     close(sockfd);

//     return 0;
// }
