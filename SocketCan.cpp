#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <linux/can.h>
#include <linux/if.h>
#include <sys/ioctl.h>
#include <linux/can/raw.h>

#define FRAME_ID 0x555
#define FRAME_DATA "Hello"

int main() {
    // Create CAN socket
    int can_socket_fd = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (can_socket_fd == -1) {
        perror("Failed to create CAN socket");
        return 1;
    }
    std::cout << "CAN socket created successfully" << std::endl;

    // Get interface information
    struct ifreq interface_request;
    std::strcpy(interface_request.ifr_name, "vcan0");
    if (ioctl(can_socket_fd, SIOCGIFINDEX, &interface_request) == -1) {
        perror("Failed to get interface information");
        close(can_socket_fd);
        return 1;
    }
    std::cout << "Interface information retrieved successfully" << std::endl;

    // Connect to CAN interface
    struct sockaddr_can can_address;
    std::memset(&can_address, 0, sizeof(can_address));
    can_address.can_family = AF_CAN;
    can_address.can_ifindex = interface_request.ifr_ifindex;
    if (bind(can_socket_fd, reinterpret_cast<struct sockaddr*>(&can_address), sizeof(can_address)) == -1) {
        perror("Failed to connect to CAN interface");
        close(can_socket_fd);
        return 1;
    }
    std::cout << "Connected to CAN interface successfully" << std::endl;

    // Create and send CAN frame
    struct can_frame frame;
    frame.can_id = FRAME_ID;
    frame.can_dlc = sizeof(FRAME_DATA) - 1;
    std::memcpy(frame.data, FRAME_DATA, frame.can_dlc);
    int num_bytes_sent = write(can_socket_fd, &frame, sizeof(frame));
    if (num_bytes_sent != sizeof(frame)) {
        perror("Failed to send CAN frame");
        close(can_socket_fd);
        return 1;
    }
    std::cout << "CAN frame sent successfully" << std::endl;

    // Read CAN frame
    struct can_frame read_frame;
    int num_bytes_received = read(can_socket_fd, &read_frame, sizeof(read_frame));
    if (num_bytes_received < 0) {
        perror("Failed to read CAN frame");
        close(can_socket_fd);
        return 1;
    }
    std::printf("CAN frame received: 0x%03X [%d] ", read_frame.can_id, read_frame.can_dlc);
    for (int i = 0; i < read_frame.can_dlc; i++) {
        std::printf("%02X ", read_frame.data[i]);
    }
    std::printf("\r\n");

    // Set filter for CAN frames
    struct can_filter filter[1];
    filter[0].can_id   = 0x550;
    filter[0].can_mask = CAN_SFF_MASK;
    if (setsockopt(can_socket_fd, SOL_CAN_RAW, CAN_RAW_FILTER, &filter, sizeof(filter)) == -1) {
        perror("Failed to set CAN frame filter");
        close(can_socket_fd);
        return 1;
    }
    std::cout << "CAN frame filter set successfully" << std::endl;

    // Close CAN socket
    if (close(can_socket_fd) == -1) {
        perror("Failed to close CAN socket");
        return 1;
    }
    std::cout << "CAN frame disconnect successfully" << std::endl;
}
