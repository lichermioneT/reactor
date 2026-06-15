#pragma once
#include "dispatcher.h"

struct eventloop
{
  struct dispatcher* dispatch;
  void* dispatcherdata;
};
