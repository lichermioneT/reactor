#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include "HttpRequest.h"
#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include "TcpConnection.h"
#include <assert.h>
#include <ctype.h>

char* HttpRequest::splitRequestLine(const char* start, const char* end, const char* sub, function<void(string)> callback)
{
    if (start == nullptr || end == nullptr || start > end)
    {
        return nullptr;
    }
    char* space = const_cast<char*>(end);
    if (sub != nullptr)
    {
        space = static_cast<char*>(memmem(start, end - start, sub, strlen(sub)));
        if (space == nullptr)
        {
            return nullptr;
        }
    }
    int length = space - start;
    callback(string(start, length));
    return sub == nullptr ? space : space + strlen(sub);
}


// 将字符转换为整形数
int HttpRequest::hexToDec(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;

    return 0;
}

HttpRequest::HttpRequest()
{
    reset();
}

HttpRequest::~HttpRequest()
{
}

void HttpRequest::reset()
{
    _curState = PrecessState::ParseReqLine;
    _method = _url = _version = string(); // ""
    _reqHeaders.clear();
}

void HttpRequest::addHeader(const string key, const string value)
{
    if (key.empty() || value.empty())
    {
        return;
    }
    _reqHeaders.insert(make_pair(key, value));
}

string HttpRequest::getHeader(const string key)
{
    auto item = _reqHeaders.find(key);
    if (item == _reqHeaders.end())
    {
        return string();
    }
    return item->second;
}

bool HttpRequest::parseRequestLine(Buffer* readBuf)
{
    // 读出请求行, 保存字符串结束地址
    char* end = readBuf->findCRLF();
    if (end == nullptr)
    {
        return false;
    }
    // 保存字符串起始地址
    char* start = readBuf->data();
    // 请求行总长度
    int lineSize = end - start;

    if (lineSize > 0)
    {
        auto methodFunc = bind(&HttpRequest::setMethod, this, placeholders::_1);
        start = splitRequestLine(start, end, " ", methodFunc);
        if (start == nullptr)
        {
            return false;
        }
        auto urlFunc = bind(&HttpRequest::seturl, this, placeholders::_1);
        start = splitRequestLine(start, end, " ", urlFunc);
        if (start == nullptr)
        {
            return false;
        }
        auto versionFunc = bind(&HttpRequest::setVersion, this, placeholders::_1);
        if (splitRequestLine(start, end, nullptr, versionFunc) == nullptr)
        {
            return false;
        }
        // 为解析请求头做准备
        readBuf->readPosIncrease(lineSize + 2);
        // 修改状态
        setState(PrecessState::ParseReqHeaders);
        return true;
    }
    return false;
}

bool HttpRequest::parseRequestHeader(Buffer* readBuf)
{
    char* end = readBuf->findCRLF();
    if (end != nullptr)
    {
        char* start = readBuf->data();
        int lineSize = end - start;
        // 基于: 搜索字符串
        char* middle = static_cast<char*>(memmem(start, lineSize, ": ", 2));
        if (middle != nullptr)
        {
            int keyLen = middle - start;
            int valueLen = end - middle - 2;
            if (keyLen > 0 && valueLen > 0)
            {
                string key(start, keyLen);
                string value(middle + 2, valueLen);
                addHeader(key, value);
            }
            // 移动读数据的位置
            readBuf->readPosIncrease(lineSize + 2);
        }
        else
        {
            // 请求头被解析完了, 跳过空行
            readBuf->readPosIncrease(2);
            // 修改解析状态
            // 忽略 post 请求, 按照 get 请求处理
            setState(PrecessState::ParseReqDone);
        }
        return true;
    }
    return false;
}

bool HttpRequest::parseHttpRequest(Buffer* readBuf, HttpResponse* response, Buffer* sendBuf, int socket)
{
    bool flag = true;
    while (_curState != PrecessState::ParseReqDone)
    {
        switch (_curState)
        {
        case PrecessState::ParseReqLine:
            flag = parseRequestLine(readBuf);
            break;
        case PrecessState::ParseReqHeaders:
            flag = parseRequestHeader(readBuf);
            break;
        case PrecessState::ParseReqBody:
            break;
        default:
            break;
        }
        if (!flag)
        {
            return false;
        }
        if (_curState == PrecessState::ParseReqDone)
        {
            if (!processHttpRequest(response))
            {
                return false;
            }
            response->prepareMsg(sendBuf, socket);
        }
    }
    reset();
    return flag;
}

bool HttpRequest::processHttpRequest(HttpResponse* response)
{
    if (strcasecmp(_method.c_str(), "get") != 0)
    {
        return false;
    }
    _url = decodeMsg(_url);
    // 处理客户端请求的静态资源(目录或者文件)
    const char* file = NULL;
    if (strcmp(_url.c_str(), "/") == 0)
    {
        file = "./";
    }
    else
    {
        file = _url.c_str() + 1;
    }
    // 获取文件属性
    struct stat st;
    int ret = stat(file, &st);
    if (ret == -1)
    {
        // 文件不存在 -- 回复404
        //sendHeadMsg(cfd, 404, "Not Found", getFileType(".html"), -1);
        //sendFile("404.html", cfd);
        response->setFileName("404.html");
        response->setStatusCode(StatusCode::NotFound);
        // 响应头
        response->addHeader("Content-type", getFileType(".html"));
        response->sendDataFunc = sendFile;
        return true;
    }

    response->setFileName(file);
    response->setStatusCode(StatusCode::OK);
    // 判断文件类型
    if (S_ISDIR(st.st_mode))
    {
        // 把这个目录中的内容发送给客户端
        //sendHeadMsg(cfd, 200, "OK", getFileType(".html"), -1);
        //sendDir(file, cfd);
        // 响应头
        response->addHeader("Content-type", getFileType(".html"));
        response->sendDataFunc = sendDir;
    }
    else
    {
        // 把文件的内容发送给客户端
        //sendHeadMsg(cfd, 200, "OK", getFileType(file), st.st_size);
        //sendFile(file, cfd);
        // 响应头
        response->addHeader("Content-type", getFileType(file));
        response->addHeader("Content-length", to_string(st.st_size));
        response->sendDataFunc = sendFile;
    }

    return true;
}

string HttpRequest::decodeMsg(const string& msg)
{
    string str = string();
    const char* from = msg.c_str();
    for (; *from != '\0'; ++from)
    {
        // isxdigit -> 判断字符是不是16进制格式, 取值在 0-f
        // Linux%E5%86%85%E6%A0%B8.jpg
        if (from[0] == '%' && isxdigit(from[1]) && isxdigit(from[2]))
        {
            // 将16进制的数 -> 十进制 将这个数值赋值给了字符 int -> char
            // B2 == 178
            // 将3个字符, 变成了一个字符, 这个字符就是原始数据
            str.append(1, hexToDec(from[1]) * 16 + hexToDec(from[2]));

            // 跳过 from[1] 和 from[2] 因此在当前循环中已经处理过了
            from += 2;
        }
        else
        {
            // 字符拷贝, 赋值
            str.append(1, *from);
        }
    }
    return str;
}

const string HttpRequest::getFileType(const string& name)
{
    // a.jpg a.mp4 a.html
    // 自右向左查找‘.’字符, 如不存在返回NULL
    const char* dot = strrchr(name.c_str(), '.');
    if (dot == NULL)
        return "text/plain; charset=utf-8";	// 纯文本
    if (strcmp(dot, ".html") == 0 || strcmp(dot, ".htm") == 0)
        return "text/html; charset=utf-8";
    if (strcmp(dot, ".jpg") == 0 || strcmp(dot, ".jpeg") == 0)
        return "image/jpeg";
    if (strcmp(dot, ".gif") == 0)
        return "image/gif";
    if (strcmp(dot, ".png") == 0)
        return "image/png";
    if (strcmp(dot, ".css") == 0)
        return "text/css";
    if (strcmp(dot, ".au") == 0)
        return "audio/basic";
    if (strcmp(dot, ".wav") == 0)
        return "audio/wav";
    if (strcmp(dot, ".avi") == 0)
        return "video/x-msvideo";
    if (strcmp(dot, ".mov") == 0 || strcmp(dot, ".qt") == 0)
        return "video/quicktime";
    if (strcmp(dot, ".mpeg") == 0 || strcmp(dot, ".mpe") == 0)
        return "video/mpeg";
    if (strcmp(dot, ".vrml") == 0 || strcmp(dot, ".wrl") == 0)
        return "model/vrml";
    if (strcmp(dot, ".midi") == 0 || strcmp(dot, ".mid") == 0)
        return "audio/midi";
    if (strcmp(dot, ".mp3") == 0)
        return "audio/mpeg";
    if (strcmp(dot, ".ogg") == 0)
        return "application/ogg";
    if (strcmp(dot, ".pac") == 0)
        return "application/x-ns-proxy-autoconfig";

    return "text/plain; charset=utf-8";
}

void HttpRequest::sendDir(const string& dirName, Buffer* sendBuf, int cfd)
{
    char buf[4096] = { 0 };
    snprintf(buf, sizeof(buf), "<html><head><title>%s</title></head><body><table>", dirName.c_str());
    struct dirent** namelist = nullptr;
    int num = scandir(dirName.c_str(), &namelist, NULL, alphasort);
    if (num < 0)
    {
        perror("scandir");
        return;
    }
    for (int i = 0; i < num; ++i)
    {
        // 取出文件名 namelist 指向的是一个指针数组 struct dirent* tmp[]
        char* name = namelist[i]->d_name;
        struct stat st;
        char subPath[1024] = { 0 };
        snprintf(subPath, sizeof(subPath), "%s/%s", dirName.c_str(), name);
        if (stat(subPath, &st) == -1)
        {
            free(namelist[i]);
            continue;
        }
        if (S_ISDIR(st.st_mode))
        {
            // a标签 <a href="">name</a>
            snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf),
                "<tr><td><a href=\"%s/\">%s</a></td><td>%ld</td></tr>",
                name, name, st.st_size);
        }
        else
        {
            snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf),
                "<tr><td><a href=\"%s\">%s</a></td><td>%ld</td></tr>",
                name, name, st.st_size);
        }
        // send(cfd, buf, strlen(buf), 0);
        sendBuf->appendString(buf);
#ifndef MSG_SEND_AUTO
        sendBuf->sendData(cfd);
#endif
        memset(buf, 0, sizeof(buf));
        free(namelist[i]);
    }
    snprintf(buf, sizeof(buf), "</table></body></html>");
    // send(cfd, buf, strlen(buf), 0);
    sendBuf->appendString(buf);
#ifndef MSG_SEND_AUTO
    sendBuf->sendData(cfd);
#endif
    free(namelist);
}

void HttpRequest::sendFile(const string& fileName, Buffer* sendBuf, int cfd)
{
    // 1. 打开文件
    int fd = open(fileName.c_str(), O_RDONLY);
    if (fd == -1)
    {
        perror("open");
        return;
    }
#if 1
    while (1)
    {
        char buf[1024];
        int len = read(fd, buf, sizeof buf);
        if (len > 0)
        {
            // send(cfd, buf, len, 0);
            sendBuf->appendString(buf, len);
#ifndef MSG_SEND_AUTO
            sendBuf->sendData(cfd);
#endif
        }
        else if (len == 0)
        {
            break;
        }
        else
        {
            perror("read");
            break;
        }
    }
#else
    off_t offset = 0;
    int size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    while (offset < size)
    {
        int ret = sendfile(cfd, fd, &offset, size - offset);
        printf("ret value: %d\n", ret);
        if (ret == -1 && errno == EAGAIN)
        {
            printf("没数据...\n");
        }
    }
#endif
    close(fd);
}
