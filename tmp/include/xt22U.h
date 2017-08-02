#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_


#define RT_OK					0x3000 //成功
#define RT_FAIL   				0x30E0 //失败
#define PACKET_ERROR			0x30E1 //包收发错误
#define NO_CONTROL				0x30E2 //没权限
#define NO_DETECT_FINGER		0x30E3 //传感器上没探测到手指
#define	FULL_FINGER    		0x30E4  //手指已经录入满了
#define PARAM_ERROR			0x30E5 //参数错误
#define TIME_OUT				0x30E6  //超时
#define COMMAND_ERROR 		0x30E7 //命令错误
#define NO_USER	    			0x30E8 //没用户
#define DATA_NOT_EXIST 		0x30E9//数据不存在

#define FPOK 0x3000

//打开设备
extern	BOOL WINAPI FU_OpenDevice(int nDevNum,HANDLE* phHandle);
//关闭设备 
extern	BOOL WINAPI FU_CloseDevice(HANDLE hHandle);
//获取错误信息
extern	int WINAPI FU_GetErrorString(int nError,char* strError);
//复位设备 
extern	int WINAPI FU_ResetDevice(HANDLE hHandle);
//从设备获取图像，图像缓冲区pImagData为152*200字节的unsigned char指针
extern	int WINAPI FU_GetFpImage(HANDLE hHandle,unsigned char *pImgData);
//从设备获取图像后，再写入BMP格式文件，strFileName为指向存放路径的指针，pPicData为指向图像数据的指针，大小为152*200
extern  BOOL WINAPI FU_MakeBMPFile(char* strFileName,unsigned char *pPicData);
 	 
extern int WINAPI FU_IsFingerOn(HANDLE hHandle, int*fingerStatus);

#define PUBLIC_INFO 1
#define PRIVATE_INFO 2

#endif