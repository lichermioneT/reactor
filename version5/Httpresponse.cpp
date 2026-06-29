#include "HttpResponse.h"
#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

HttpResponse::HttpResponse()
{
    _statusCode = StatusCode::Unknown;
    _headers.clear();
    _fileName = string();
    sendDataFunc = nullptr;
}

HttpResponse::~HttpResponse()
{
}

void HttpResponse::addHeader(const string key, const string value)
{
    if (key.empty() || value.empty())
    {
        return;
    }
    _headers.insert(make_pair(key, value));
}

void HttpResponse::prepareMsg(Buffer* sendBuf, int socket)
{
    // 状态行
    char tmp[1024] = { 0 };
    int code = static_cast<int>(_statusCode);
    auto info = _info.find(code);
    if (info == _info.end())
    {
        code = static_cast<int>(StatusCode::BadRequest);
        info = _info.find(code);
    }
    snprintf(tmp, sizeof(tmp), "HTTP/1.1 %d %s\r\n", code, info->second.c_str());
    sendBuf->appendString(tmp);
    // 响应头
    for (auto it = _headers.begin(); it != _headers.end(); ++it)
    {
        snprintf(tmp, sizeof(tmp), "%s: %s\r\n", it->first.c_str(), it->second.c_str());
        sendBuf->appendString(tmp);
    }
    // 空行
    sendBuf->appendString("\r\n");
#ifndef MSG_SEND_AUTO
    sendBuf->sendData(socket);
#endif

    // 回复的数据
    if (sendDataFunc)
    {
        sendDataFunc(_fileName, sendBuf, socket);
    }
}
