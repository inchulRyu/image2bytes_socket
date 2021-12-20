#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/uio.h>
#include <vector>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

#include <iostream>

using namespace std;

#define PORT 20162
#define BUFFER_SIZE 4096

void matToBytes(cv::Mat & image, unsigned char * bytes)
{
   int size = image.total() * image.elemSize();
   cout << "size : " << size << endl;
   bytes = new unsigned char[size];  // you will have to delete[] that later // 이거 때문에 에러가 난듯.
//    bytes = (unsigned char *) calloc(size, sizeof(unsigned char));
   std::memcpy(bytes, image.data, size * sizeof(unsigned char));
}

int main(int argc, char** argv)
{
    // 이미지 로드
    cv::Mat img1 = cv::imread("./cat1.png", cv::IMREAD_UNCHANGED);
    // unsigned char * bytes;

    // // 바이트로 변경
    // matToBytes(img1, bytes);

    // 인코딩하기
    std::vector<unsigned char> byte_img;
    cv::imencode(".jpeg", img1, byte_img);
    cout << "byte_img.size() : " << byte_img.size() << endl;

    // std::vector<unsigned char> send_vec {1,2,3,4,5,6,7,8,9,10};

    // for (int i = 0; i < send_vec.size(); i++){
    //     std::cout << static_cast<unsigned>(send_vec[i]) << " ";
    // }
    // std::cout << "\n";

    struct sockaddr_in connectSocket;

    memset(&connectSocket, 0, sizeof(connectSocket));

    connectSocket.sin_family = AF_INET;
    inet_aton("127.0.0.1", (struct in_addr*) &connectSocket.sin_addr.s_addr);
    connectSocket.sin_port = htons(PORT);

    int connectFD = socket(AF_INET, SOCK_STREAM, 0);

    if (connect(connectFD, (struct sockaddr*) &connectSocket, sizeof(connectSocket)) == -1)
    {
        printf("Can not connect.\n");
        return -1;
    }
    else
    {
        int read_bytes, written_bytes;
        char receiveBuffer[BUFFER_SIZE];

        read_bytes = read(connectFD, receiveBuffer, BUFFER_SIZE);
        receiveBuffer[read_bytes] = '\0';
        cout << receiveBuffer << endl;

        // 서버에 데이터 보내기
        // written_bytes = write(connectFD, send_buffer, strlen(send_buffer));
        // written_bytes = write(connectFD, send_buffer, BUFFER_SIZE);
        // written_bytes = write(connectFD, send_vec.data(), send_vec.size());
        written_bytes = write(connectFD, byte_img.data(), byte_img.size());
        cout << written_bytes << " bytes write" << endl;
    }

    close(connectFD);

    return 0;
}