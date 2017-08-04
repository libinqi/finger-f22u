#include "F22U.h"
#include <iostream>
#include <string>
#include <stdio.h>
#include <windows.h>

using namespace v8;

Persistent<Function> F22U::constructor;

int featureTimes;                    // 指纹特征值比对次数(2次或3次)
int hasFeatureTimes;                 // 指纹特征值已比对次数
int imageScores;                     // 指纹图像分数
int featurePoints;                   // 指纹特征值点数
int encrollScores;                   // 指纹比对分数
HANDLE m_hDevice;                    //设备句柄
unsigned char m_pImgData[256 * 288]; // 指纹图片数据
unsigned char m_pMbData[512];        // 指纹模板数据

unsigned char featureData1[512];
unsigned char featureData2[512];
unsigned char featureData3[512];

F22U::F22U()
{
    m_hDevice = INVALID_HANDLE_VALUE;
    doExecute = execute;
}

F22U::~F22U()
{
    if (m_hDevice == INVALID_HANDLE_VALUE)
    {
        FU_CloseDevice(m_hDevice);
        m_hDevice = INVALID_HANDLE_VALUE;
    }
}

void F22U::Init(Handle<Object> exports)
{
    Isolate *isolate = Isolate::GetCurrent();

    // Prepare constructor template
    Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
    tpl->SetClassName(String::NewFromUtf8(isolate, "F22U"));
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    // Prototype
    NODE_SET_PROTOTYPE_METHOD(tpl, "Open", Open);
    NODE_SET_PROTOTYPE_METHOD(tpl, "Start", Start);
    NODE_SET_PROTOTYPE_METHOD(tpl, "Pause", Pause);
    NODE_SET_PROTOTYPE_METHOD(tpl, "Close", Close);

    constructor.Reset(isolate, tpl->GetFunction());
    exports->Set(String::NewFromUtf8(isolate, "F22U"),
                 tpl->GetFunction());
}

void F22U::New(const FunctionCallbackInfo<Value> &args)
{
    Isolate *isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);

    if (args.IsConstructCall())
    {
        // Invoked as constructor: `new MyObject(...)`

        F22U *obj = new F22U();
        obj->Wrap(args.This());
        args.GetReturnValue().Set(args.This());
    }
    else
    {
        // Invoked as plain function `MyObject(...)`, turn into construct call.
        const int argc = 1;
        Local<Value> argv[argc] = {args[0]};
        Local<Function> cons = Local<Function>::New(isolate, constructor);
        args.GetReturnValue().Set(cons->NewInstance(argc, argv));
    }
}

bool F22U::getFingerImage()
{
    int nFlag = RT_FAIL;
    int fingerStatus = 0;

    FU_IsFingerOn(m_hDevice, &fingerStatus);

    if (fingerStatus == 1)
    {
        return false;
    }

    nFlag = FU_GetFpImage(m_hDevice, m_pImgData);
    if (nFlag != RT_OK)
    {
        return false;
    }

    return true;
}

bool F22U::getFingerFeature()
{
    int _imageScores = 0;
    int _featurePoints = 0;

    if (hasFeatureTimes <= 0)
    {
        hasFeatureTimes = featureTimes;
    }

    if (hasFeatureTimes == 3)
    {
        if (0 == GenChar(m_pImgData, featureData3))
        {
            _imageScores = featureData3[2];
            _featurePoints = featureData3[3];
        }
    }
    if (hasFeatureTimes == 2)
    {
        if (0 == GenChar(m_pImgData, featureData2))
        {
            _imageScores = featureData2[2];
            _featurePoints = featureData2[3];
        }
    }
    if (hasFeatureTimes == 1)
    {
        if (0 == GenChar(m_pImgData, featureData1))
        {
            _imageScores = featureData1[2];
            _featurePoints = featureData1[3];
        }
    }

    if (_imageScores < imageScores || _featurePoints < featurePoints)
    {
        return false;
    }

    hasFeatureTimes--;

    return true;
}

bool F22U::getEncrollTemplate()
{
    if (hasFeatureTimes == 0)
    {
        int featureScore1 = 0;
        int featureScore2 = 0;
        int featureScore3 = 0;

        if (featureTimes >= 3)
        {
            int featureScore1 = Match2Fp(featureData1, featureData2);
            int featureScore2 = Match2Fp(featureData1, featureData3);
            int featureScore3 = Match2Fp(featureData2, featureData3);

            if (featureScore1 > encrollScores && featureScore2 > encrollScores && featureScore3 > encrollScores)
            {
                switch (featureScore1 < featureScore2
                            ? (featureScore1 < featureScore3 ? 0 : 2)
                            : (featureScore2 < featureScore3 ? 1 : 2))
                {
                case 0:
                    memcpy(m_pMbData, featureData1, 256);
                    memcpy(m_pMbData + 256, featureData2, 256);
                    break;
                case 1:
                    memcpy(m_pMbData, featureData1, 256);
                    memcpy(m_pMbData + 256, featureData3, 256);
                    break;
                case 2:
                    memcpy(m_pMbData, featureData2, 256);
                    memcpy(m_pMbData + 256, featureData3, 256);
                    break;
                }
                return true;
            }
        }
        else if (featureTimes == 1)
        {
            memcpy(m_pMbData, featureData1, sizeof(featureData1));

            if (m_pMbData[0] != 0x00)
            {
                return true;
            }
        }
        else
        {
            int featureScore1 = Match2Fp(featureData1, featureData2);

            if (featureScore1 > encrollScores)
            {
                memcpy(m_pMbData, featureData1, 256);
                memcpy(m_pMbData + 256, featureData2, 256);

                if (m_pMbData[0] != 0x00)
                {
                    return true;
                }
            }
        }
    }
    return false;
}

void F22U::Open(const FunctionCallbackInfo<Value> &args)
{
    Isolate *isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);

    int result = 0;
    if (m_hDevice != INVALID_HANDLE_VALUE)
        FU_CloseDevice(m_hDevice);

    m_hDevice = INVALID_HANDLE_VALUE;

    if (FU_OpenDevice(0, &m_hDevice))
    {
        if (FU_ResetDevice(m_hDevice) == RT_OK)
            result = 1;
    }

    bind(isolate, Local<Function>::Cast(args[0]), Local<Function>::Cast(args[1]));

    args.GetReturnValue().Set(Number::New(isolate, result));
}

void F22U::Start(const FunctionCallbackInfo<Value> &args)
{
    Isolate *isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);

    hasFeatureTimes = featureTimes = args[0]->Int32Value();
    imageScores = args[1]->Int32Value();
    featurePoints = args[2]->Int32Value();
    encrollScores = args[3]->Int32Value();

    memset(featureData1, 0x00, sizeof(featureData1));
    memset(featureData2, 0x00, sizeof(featureData2));
    memset(featureData3, 0x00, sizeof(featureData3));

    resume();

    args.GetReturnValue().Set(Number::New(isolate, 1));
}

void F22U::Pause(const FunctionCallbackInfo<Value> &args)
{
    Isolate *isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);

    suspend();

    args.GetReturnValue().Set(Number::New(isolate, 1));
}

void F22U::Close(const FunctionCallbackInfo<Value> &args)
{
    Isolate *isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);

    bool result = false;

    if (m_hDevice != INVALID_HANDLE_VALUE)
    {
        result = FU_CloseDevice(m_hDevice);
        m_hDevice = INVALID_HANDLE_VALUE;
    }

    args.GetReturnValue().Set(Number::New(isolate, result ? 1 : 0));
}

void F22U::execute()
{
    if (getFingerImage())
    {
        // char picBufData[256 * 288];
        // if (FU_MakeBMPFile("finger_image.bmp", m_pImgData))
        // {
        //     FILE *fp = fopen("finger_image.bmp", "r");
        //     if (fp)
        //     {
        //         fread(picBufData, 1, sizeof(picBufData), fp);
        //         fclose(fp);
        //     }
        // }
        // memcpy(receiver.fingerImageData, picBufData, sizeof(picBufData));

        // FU_MakeBMPFile("finger_image.bmp", m_pImgData);

        Receiver receiver;
        receiver.type = ReceiveType::Image;

        memcpy(receiver.fingerImageData, m_pImgData, sizeof(m_pImgData));

        if (getFingerFeature())
        {
            receiver.isHeightImage = 1;
            doReceive(receiver);

            if (getEncrollTemplate())
            {
                suspend();

                receiver.type = ReceiveType::Feature;

                memcpy(receiver.fingerTemplateData, m_pMbData, sizeof(m_pMbData));
                doReceive(receiver);
            }
        }
        else
        {
            receiver.isHeightImage = 0;
            doReceive(receiver);
        }
    }
}

void InitAll(Handle<Object> exports)
{
    F22U::Init(exports);
}

NODE_MODULE(F22U, InitAll);