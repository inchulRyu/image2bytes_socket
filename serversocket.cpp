#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/types.h>
#include <queue>
#include <vector>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

#include <typeinfo>
 
#define PORT 20162
#define BUFFER_SIZE 4096

#define OBJ_DATA_SIZE 8
#define CAM_ADDRESS_SIZE 200

# define LISTEN_QUEUE_SIZE 5

using std::vector;
using std::thread;
using std::cout;
using std::endl;
using std::string;

int client(int connectFD, struct sockaddr_in connectSocket,
            // socklen_t connectSocketLength, 
            vector<thread*> &clientlist, std::mutex &client_thread_m, std::mutex &client_data_m)
{
    struct sockaddr_in peerSocket;

    ssize_t received_bytes, written_bytes;
    char read_buffer[BUFFER_SIZE];
    char send_buffer[BUFFER_SIZE];

    memset(read_buffer, 0, BUFFER_SIZE);
    memset(send_buffer, 0, BUFFER_SIZE);

    // getpeername(connectFD, (struct sockaddr*)&peerSocket, &connectSocketLength);
    // char peerName[sizeof(peerSocket.sin_addr) + 1] = { 0 };
    // sprintf(peerName, "%s", inet_ntoa(peerSocket.sin_addr));
    // // 접속이 안되었을 때는 출력 x
    // if(strcmp(peerName,"0.0.0.0") != 0)
    //     printf("Client : %s\n", peerName);

    char client_ip[30];
    uint16_t client_port;

    strcpy(client_ip, inet_ntoa(connectSocket.sin_addr));
    client_port = ntohs(connectSocket.sin_port);

    // // 접속 정보 콘솔에 출력
    // cout << "\n=======================================================================================" << endl;
    // cout << "Client connected IP address = " << inet_ntoa(connectSocket.sin_addr) << ":" << ntohs(connectSocket.sin_port) << endl;
    // cout << "=======================================================================================\n" << endl;

    const char* message = "Welcome server!\r\n\0";
    send(connectFD, message, strlen(message)+1, 0);
    
    if (connectFD < 0)
    {
        printf("Server: accept failed\n");
        return 0;
    }

    // vector<char> result_vec;
    vector<unsigned char> result_vec;
    int data_cnt = 0;
    int total_length = 223595;

    // read할 값이 있다면 계속 읽어들인다.
    while((received_bytes = read(connectFD, read_buffer, BUFFER_SIZE)) > 0)
    {
        if (data_cnt >= total_length) continue;
        // cout << received_bytes << " bytes read" << endl; 

        // char * result = new char[received_bytes+1];

        int buffer_cursor = 0;

        int read_length = 0;

        if (total_length - data_cnt - received_bytes < 0){
            read_length = total_length - data_cnt;
        }else{
            read_length = received_bytes;
        }

        std::copy(read_buffer, read_buffer + read_length, std::back_inserter(result_vec));
        data_cnt += read_length;
        // cout << "added : " << read_length << endl;

        // cout << "read buffer size : " << sizeof(read_buffer) << endl;
    }

    // // 벡터 프린트
    // for (auto ch : result_vec){
    //     cout << static_cast<unsigned>(ch) << " ";
    // }
    // cout << "\n";

    cout << "result_vec size : " << result_vec.size() << endl;

    std::ostringstream thread_id_ss;
    thread_id_ss << std::this_thread::get_id();
    std::string thread_id_str = thread_id_ss.str();
    cout << "thread id : " << thread_id_str << endl;
    std::string window_name = "image" + thread_id_str;

    cv::Mat decoded_img = cv::imdecode(result_vec, cv::IMREAD_COLOR);

    // // 쓰레드별로 이미지를 따로 띄우도록 해보려고했으나 실패함. 
    // // 그래서 그냥 mutex로 한번에 하나의 이미지만 뜨도록 함.
    // 이미지 띄우기
    client_data_m.lock();
    cv::namedWindow(window_name);
    cv::imshow(window_name, decoded_img);
    cv::waitKey(2000);

    cv::destroyWindow(window_name);
    decoded_img.release();
    client_data_m.unlock();

    // 클라이언트가 종료되면 커넥팅 소켓도 종료
    close(connectFD); 
    // 접속 해제 정보 콘솔에 출력
    cout << "\n=======================================================================================" << endl;
    cout << "Client disconnected IP address = " << inet_ntoa(connectSocket.sin_addr) << ":" << ntohs(connectSocket.sin_port) << endl;
    cout << "=======================================================================================\n" << endl;

    // thread_list 에서 현재 쓰레드를 제거한다.
    client_thread_m.lock();
    for (auto ptr = clientlist.begin(); ptr < clientlist.end(); ptr++){
        // thread 아이디가 같은 것을 찾아서
        if ((*ptr)->get_id() == std::this_thread::get_id()){
            clientlist.erase(ptr); // 쓰레드가 끝나면 알아서 할당된 메모리가 사라지므로 따로 delete 할 필요가 없이 목록에서 지워주기만 하면 된다.
            break;
        }
    }
    client_thread_m.unlock();

    return 0;
}
 
// int server_socket(std::queue<Object_stat*> &recv_data_queue, std::vector<Client_data *> &client_data_list, std::mutex &client_data_m)
int main()
{
    vector<thread*> clientlist;
 
    struct sockaddr_in listenSocket;
 
    memset(&listenSocket, 0, sizeof(listenSocket));
 
    listenSocket.sin_family = AF_INET;
    listenSocket.sin_addr.s_addr = htonl(INADDR_ANY);
    listenSocket.sin_port = htons(PORT);
 
    int listenFD = socket(AF_INET, SOCK_STREAM, 0);
    int connectFD;
 
    if (bind(listenFD, (struct sockaddr *) &listenSocket, sizeof(listenSocket)) == -1) {
        printf("Can not bind.\n");
        return -1;
    }
 
    if (listen(listenFD, LISTEN_QUEUE_SIZE) == -1) {
        printf("Listen fail.\n");
        return -1;
    }
 
    printf("Server Start!\n");

    std::mutex client_thread_m;
    std::mutex client_data_m;
 
    while (1) 
    {
        struct sockaddr_in connectSocket;
        socklen_t connectSocketLength = sizeof(connectSocket);
 
        while((connectFD = accept(listenFD, (struct sockaddr*)&connectSocket, (socklen_t *)&connectSocketLength)) >= 0)
        {
            // 클라이언트 리스트에 새 클라이언트 쓰레드 추가 및 실행.
            client_thread_m.lock();
            clientlist.push_back(new thread(client, connectFD, connectSocket, std::ref(clientlist), std::ref(client_thread_m), std::ref(client_data_m)));
            // clientlist.push_back(new thread(client, connectFD, connectSocket));
            client_thread_m.unlock();
        }
    }
    close(listenFD);

    return 0;
}
