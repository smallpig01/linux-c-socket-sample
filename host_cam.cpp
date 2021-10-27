#include <iostream>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define SOCKET_ADDRESS_FAMILY PF_INET
#define SOCKET_ADDRESS_PORT 8088
#define NUM_BUFFER_TO_SEND 50
#define IMAGE_BUFFER_SIZE (1280 * 801 * 3)

using namespace std;

#include <sys/time.h>
double GetTick()
{
    timeval t;
    gettimeofday(&t, NULL);
    return t.tv_sec * 1000 + t.tv_usec / 1000;
}

/// change image content by first byte value+5
int FakeImgAdder(char *data, int len)
{
    char new_value = *data + 5;
    memset(data, new_value, len);
    return new_value;
}

int main()
{
    // generate fake depth img for send()
    int img_buff_len = IMAGE_BUFFER_SIZE;
    char *fake_img = new char[img_buff_len];
    memset(fake_img, 0, img_buff_len);

    // create a host socket
    int host_fd = 0;
    host_fd = socket(SOCKET_ADDRESS_FAMILY, SOCK_STREAM, 0);
    if (host_fd == -1)
    {
        printf("Fail to create host socket.");
    }

    // use PF_INET socket
    struct sockaddr_in host_info;
    struct sockaddr_in client_info;
    socklen_t addrlen = sizeof(client_info);
    bzero(&host_info, sizeof(host_info));
    host_info.sin_family = SOCKET_ADDRESS_FAMILY;
    host_info.sin_addr.s_addr = INADDR_ANY;
    host_info.sin_port = htons(SOCKET_ADDRESS_PORT);

    // set socket as reuse and retry bind()
    int opt = 1;
    int opt_set_result = -1;
    opt_set_result = setsockopt(host_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if (opt_set_result == -1)
    {
        printf("setsockopt SO_REUSEADDR fail : %d \n", opt_set_result);
        shutdown(host_fd, SHUT_RDWR);
        return 0;
    }

    // bind host file description and host info
    int bind_result = bind(host_fd, (struct sockaddr *)&host_info, sizeof(host_info));
    if (bind_result == -1)
    {
        printf("rebind fail : %d\n", bind_result);
        shutdown(host_fd, SHUT_RDWR);
        return 0;
    }

    // set host_fd to listen mode
    int listen_result = listen(host_fd, 5);
    if (listen_result == -1)
    {
        printf("listen fail : %d\n", listen_result);
        shutdown(host_fd, SHUT_RDWR);
        return 0;
    }

    // start wait client connect while
    double total_send_time = 0;
    int accept_index = 0;
    printf("start to accept() and send() ... \n");
    while (accept_index < NUM_BUFFER_TO_SEND)
    {
        // wait client to conntect
        int client_fd = accept(host_fd, (struct sockaddr *)&client_info, &addrlen);
        if (client_fd == -1)
        {
            printf("accept fail\n");
            shutdown(host_fd, SHUT_RDWR);
            return 0;
        }

        // send buffer
        double send_start_time = GetTick();
        long long send_size = send(client_fd, fake_img, img_buff_len, 0);
        double send_end_time = GetTick();

        // send close hand shake to host
        close(client_fd);

        double send_time = send_end_time - send_start_time;
        printf("sended: index=%ld(%ld) in %.0fms, data:%d \n", accept_index, send_size, send_time, fake_img[0]);

        // change image
        FakeImgAdder(fake_img, img_buff_len);
        total_send_time += send_time;
        ++accept_index;
    }

    printf("total_send_time = %.0fms\n", total_send_time);

    close(host_fd);
    shutdown(host_fd, SHUT_RDWR);

    return 0;
}