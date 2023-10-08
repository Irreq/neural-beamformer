#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int udpSocket;
struct sockaddr_in serverAddr;

int start_hmi()
{
    // Create a UDP socket
    udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpSocket < 0) {
        perror("Error creating socket");
        // return 1;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(54321);  // Specify the port to send to
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");  // Specify the IP address to send to

}

int stop_hmi()
{
    close(udpSocket);
}



void transmit(float *mimo, int n)
{
    sendto(udpSocket, mimo, sizeof(float) * n, 0, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
}