#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

#include <iostream>
#include <vector>

using namespace std;

void matToBytes(cv::Mat & image, unsigned char * bytes)
{
   int size = image.total() * image.elemSize();
   cout << "size : " << size << endl;
//    bytes = new unsigned char[4000000];  // you will have to delete[] that later // 이거 때문에 에러가 난듯.
//    bytes = (unsigned char *) calloc(size, sizeof(unsigned char));
   std::memcpy(bytes, image.data, size * sizeof(unsigned char));
}

void bytesToMat(unsigned char * bytes, cv::Mat & image, int width, int height)
{
    cout << "1" << endl;
    image = cv::Mat(height, width, CV_8UC4, bytes).clone(); // make a copy
    // cv::Mat image_tmp = cv::Mat(height, width, CV_8UC4, bytes); // make a copy
    cout << "1-5" << endl;
    // image = image_tmp.clone();
    cout << "2" << endl;
}

int main()
{
    // 이미지 읽기
    cv::Mat img1 = cv::imread("./cat1.png", cv::IMREAD_UNCHANGED);
    cv::Mat img2;

    int width = img1.cols;
    int height = img1.rows;

    cout << "w, h = " << width << ", " << height << endl;

    unsigned char * bytes_1;
    bytes_1 = new unsigned char[4000000];
    // unsigned char * bytes_1 = new unsigned char[4000000];
    // unsigned char * bytes_1[4000000];

    // cv::Mat test_M = cv::Mat(1000, 1000, CV_8UC4, test_1);

    // 바이트로 변경
    matToBytes(img1, bytes_1);

    // 바이트에서 다시 이미지로 변경
    bytesToMat(bytes_1, img2, width, height);

    // cout << sizeof(bytes_1) << endl;

    // 인코딩하기
    std::vector<unsigned char> byte_vec;
    cv::imencode(".jpeg", img1, byte_vec);

    cout << "vec size, capacity = " << byte_vec.size() << ", " << byte_vec.capacity() << endl;

    cv::Mat decoded_img = cv::imdecode(byte_vec, cv::IMREAD_COLOR);

    // 이미지 띄우기
    cv::namedWindow("image");
    cv::imshow("image", decoded_img);
    cv::waitKey(0);

    return 0;
}