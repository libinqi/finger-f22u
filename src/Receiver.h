#ifndef RECEIVE_H
#define RECEIVE_H

#include <string>

using namespace std;

class ReceiveType
{
  public:
    static const int Image = 1;
    static const int Feature = 2;
    static const int Verify = 3;
};

struct Receiver
{
    int type;
    int isHeightImage;
    unsigned char fingerImageData[256 * 288]; // 指纹图片数据
    unsigned char fingerTemplateData[512];        // 指纹特征数据
};

#endif // RECEIVE_H