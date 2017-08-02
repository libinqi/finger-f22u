#ifndef F22U_H
#define F22U_H
#include <uv.h>
#include <node.h>
#include <node_object_wrap.h>
#include "xt22U.h"
#include "ARTH_DLL.h"
#include "Thread.h"

class F22U : public Thread
{
public:
	static void Init(v8::Handle<v8::Object> exports);

	// private:
	// static	int featureTimes;										 // 指纹特征值比对次数(2次或3次)
	// static	int imageScores;										 // 指纹图像分数
	// static	int featurePoints;									 // 指纹特征值点数
	// static	int encrollScores;									 // 指纹比对分数
	// static HANDLE m_hDevice; //设备句柄
	// static	unsigned char m_pImgData[256 * 288]; // 指纹图片数据
	// static	unsigned char m_pMbData[512];				 // 指纹模板数据

	// static	unsigned char featureData1[512];
	// static	unsigned char featureData2[512];
	// static	unsigned char featureData3[512];

private:
	explicit F22U();
	~F22U();

	static void New(const FunctionCallbackInfo<Value> &args);
	static void Open(const FunctionCallbackInfo<Value> &args);
	static void Start(const FunctionCallbackInfo<Value> &args);
	static void Pause(const FunctionCallbackInfo<Value> &args);
	static void Close(const FunctionCallbackInfo<Value> &args);
	static v8::Persistent<v8::Function> constructor;
	static void execute();
	static bool getFingerImage();
	static bool getFingerFeature();
	static bool getEncrollTemplate();
};

#endif
