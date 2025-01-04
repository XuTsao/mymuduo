#pragma once

#include <vector>
#include <unordered_map>

#include "noncopyable.h"
#include "Timestamp.h"

class Channel;
class EventLoop;

/**
 * muduo库中多路事件分发器的核心IO复用模块
 */
class Poller : noncopyable
{
public:
    using ChannelList = std::vector<Channel*>;

    Poller(EventLoop* loop);
    virtual ~Poller() = default;

    // 给所有IO复用保留统一的接口
    virtual Timestamp poll(int timeoutMs, ChannelList *activeChannels) = 0;
    virtual void updateChannel(Channel *channel) = 0;
    virtual void removeChannel(Channel *channel) = 0;
    
    // 判断参数Channel是否在当前Poller当中
    virtual bool hasChannel(Channel *channel) const;
    // EventLoop可以通过该接口获取默认的IO复用的具体实现
    static Poller* newDefaultPoller(EventLoop *loop);
protected:
    // map的key表示fd value表示sockfd所属的channel通道类型 可以通过fd快速找到fd对应的这个封装的channel
    using ChannelMap = std::unordered_map<int, Channel*>;
    ChannelMap channels_;
private:
    EventLoop *ownerLoop_; // 定义poller所属的事件循环EventLoop
};