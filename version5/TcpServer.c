#include "TcpServer.h"

#include "TcpConnection.h"

#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

struct Listener* listenerInit(unsigned short port)
{
  struct Listener* listener = (struct Listener*)malloc(sizeof(struct Listener));
  if(listener == NULL)
  {
    perror("malloc");
    return NULL;
  }

  int listenfd = socket(AF_INET, SOCK_STREAM, 0);
  if(listenfd < 0)
  {
    perror("socket");
    free(listener);
    return NULL;
  }

  int opt = 1;
  if(setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
  {
    perror("setsockopt");
    close(listenfd);
    free(listener);
    return NULL;
  }

  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port = htons(port);

  if(bind(listenfd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
  {
    perror("bind");
    close(listenfd);
    free(listener);
    return NULL;
  }

  if(listen(listenfd, 128) < 0)
  {
    perror("listen");
    close(listenfd);
    free(listener);
    return NULL;
  }

  listener->lfd = listenfd;
  listener->port = port;
  return listener;
}

int acceptConnection(void* arg)
{
  struct TcpServer* tcpServer = (struct TcpServer*)arg;
  if(tcpServer == NULL || tcpServer->listener == NULL)
  {
    return -1;
  }

  int cfd = accept(tcpServer->listener->lfd, NULL, NULL);
  if(cfd < 0)
  {
    perror("accept");
    return -1;
  }

  struct EventLoop* evLoop = takeWorkerEventLoop(tcpServer->threadPool);
  if(evLoop == NULL)
  {
    close(cfd);
    return -1;
  }

  tcpConnectionInit(cfd, evLoop);
  return 0;
}

struct TcpServer* tcpServerInit(unsigned short port, int threadNum)
{
  struct TcpServer* tcpServer = (struct TcpServer*)malloc(sizeof(struct TcpServer));
  if(tcpServer == NULL)
  {
    perror("malloc");
    return NULL;
  }

  tcpServer->threadNum = threadNum;
  tcpServer->listener = listenerInit(port);
  tcpServer->mainLoop = EventLoopInit();
  tcpServer->threadPool = NULL;

  if(tcpServer->listener == NULL || tcpServer->mainLoop == NULL)
  {
    free(tcpServer);
    return NULL;
  }

  tcpServer->threadPool = threadPollInit(tcpServer->mainLoop, threadNum);
  if(tcpServer->threadPool == NULL)
  {
    close(tcpServer->listener->lfd);
    free(tcpServer->listener);
    free(tcpServer);
    return NULL;
  }

  return tcpServer;
}

void TcpServerRun(struct TcpServer* server)
{
  if(server == NULL || server->threadPool == NULL || server->listener == NULL ||
     server->mainLoop == NULL)
  {
    return;
  }

  threadPoolRun(server->threadPool);

  struct Channel* channel = channelInit(server->listener->lfd, ReadEvent,
                                        acceptConnection, NULL, NULL, server);
  if(channel == NULL)
  {
    return;
  }

  // MODIFIED: listener fd registration is now guarded against allocation failure.
  eventLoopAddTask(server->mainLoop, channel, ADD);
  eventLoopRun(server->mainLoop);
}
