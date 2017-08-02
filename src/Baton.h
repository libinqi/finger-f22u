#ifndef BATON_H
#define BATON_H

#include <uv.h>
#include <node.h>
#include <node_buffer.h>
#include <node_object_wrap.h>
#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <exception>
#include <windows.h>

#include "Receiver.h"
#include "Error.h"

using namespace std;
using namespace v8;

class Baton
{
public:
  uv_work_t request;
  uv_async_t asyncRequest;

  uv_mutex_t mutex;
  uv_cond_t condvar;

  Persistent<Function> onReceive;
  Persistent<Function> onError;

  bool isWaiting = false;
  int state = 0;

  // public:
  //   int featureTimes = 3;    // 指纹特征值比对次数(大于等于1次，少于等于3次)
  //   int imageScores = 75;    // 指纹图像分数
  //   int featurePoints = 15;  // 指纹特征值点数
  //   int encrollScores = 100; // 指纹比对分数

public:
  Receiver receiver;
  Error error;
};

#endif // BATON_H