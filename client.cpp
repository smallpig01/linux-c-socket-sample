#include <iostream>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define SOCKET_ADDRESS_FAMILY PF_INET
#define SOCKET_ADDRESS_PORT 8088
#define NUM_CLIENT_TO_RECEIVE 50
#define IMAGE_BUFFER_SIZE (1280 * 801 * 3)

using namespace std;

#include <sys/time.h>
double GetTick()
{
    timeval t;
    gettimeofday(&t, NULL);
    return t.tv_sec * 1000 + t.tv_usec / 1000;
}

// check buffer content is same as actual_value
int FackImageChecker(char *data, int len, char actual_value)
{
    char *p = data;
    char *end_ptr = data + len;
    int check_pos = 0;
    for (; p < end_ptr; ++p)
    {
        if (actual_value != *p)
        {
            printf("act:%d, data:%d, pos:%d\n", actual_value, *p, check_pos);
            return -1;
        }
        ++check_pos;
    }

    return actual_value;
}

int main()
{
    // generate fake depth img for recv()
    int img_buff_len = IMAGE_BUFFER_SIZE;
    char *fake_img = new char[img_buff_len];
    memset(fake_img, 255, img_buff_len);

    char buff_check_value = 0;
    int buff_err_count = 0;
    int buff_ok_count = 0;
    int recv_err_count = 0;

    double while_start_time = GetTick();

    int recv_index = 0;
    double total_recv_time = 0;
    while (recv_index < NUM_CLIENT_TO_RECEIVE)
    {

        //socket的建立
        int client_fd = 0;
        client_fd = socket(SOCKET_ADDRESS_FAMILY, SOCK_STREAM, 0);
        if (client_fd == -1)
        {
            printf("Fail to create client socket.");
        }

        unsigned int len = sizeof(unsigned int);

        //set socket param
        struct timeval recv_timeout;
        recv_timeout.tv_sec = 0;
        recv_timeout.tv_usec = 1000;
        setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&recv_timeout, sizeof(recv_timeout));

        // use PF_INET socket
        struct sockaddr_in host_info;
        bzero(&host_info, sizeof(host_info));
        host_info.sin_family = SOCKET_ADDRESS_FAMILY;
        host_info.sin_addr.s_addr = INADDR_ANY;
        host_info.sin_port = htons(SOCKET_ADDRESS_PORT);

        // connect to host
        int connect_err = connect(client_fd, (struct sockaddr *)&host_info, sizeof(host_info));
        if (connect_err == -1)
        {
            printf("Connection error, shutdown(fd)");
            close(client_fd);
            return 0;
        }

        // receive all data until buffer fill or fail
        int recved_size = 0;
        int recved_times = 0;
        double recv_start = GetTick();
        while (recved_size < img_buff_len)
        {
            ++recved_times;
            int this_recv_count = recv(client_fd, fake_img + recved_size, img_buff_len - recved_size, 0);
            if (this_recv_count <= 0)
            {
                ++recv_err_count;
                printf(" recv() error: times=%d, this=%d, total=%d\n", recved_times, this_recv_count, recved_size);
                break;
            }
            recved_size += this_recv_count;
        }
        double recv_end = GetTick();
        double recv_time = recv_end - recv_start;
        total_recv_time += recv_time;

        // check content of received buff
        int first_byte = -1;
        if (recved_size == img_buff_len)
        {
            first_byte = FackImageChecker(fake_img, img_buff_len, buff_check_value);
            if (first_byte == -1)
            {
                printf("FackImageChecker() error, image_index = %d\n", recv_index);
                ++buff_err_count;
            }
            else
            {
                ++buff_ok_count;
            }
        }

        // send close hand shake to client
        close(client_fd);

        printf("received = %d(%d), in %.1lfms(%dtimes), data=%d, recv_err=%d, img_ok=%d\n",
               recv_index, recved_size,
               recv_end - recv_start, recved_times,
               first_byte,
               recv_err_count,
               buff_ok_count);
        fflush(stdout);

        ++recv_index;
        buff_check_value += 5;
    }

    double while_end_time = GetTick();

    std::cout << "total_recv_time = " << total_recv_time << "ms" << std::endl;
    std::cout << "total_while_time = " << while_end_time - while_start_time << "ms" << std::endl;

    printf("recv_err_count = %d\n", recv_err_count);
    printf("buff_err_count = %d\n", buff_err_count);
    printf("buff_ok_count = %d\n", buff_ok_count);

    return 0;
}