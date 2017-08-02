#ifndef _ARTH_H_
#define _ARTH_H_
//比对指纹
//pSrc，pDst是要匹配的指纹特征
//返回分数，一般大于50分表示成功。也可以设置其它分数,分数越高,安全等级越高
extern int WINAPI Match2Fp(unsigned char* pSrc,unsigned char *pDst);
//提取特征
//pFingerData-指纹数据,大小为256*288,pCharData-提取出的指纹特征,256字节大小.
//返回0表示成功
extern int WINAPI GenChar(unsigned char* pFingerData,unsigned char* pCharData);
#endif