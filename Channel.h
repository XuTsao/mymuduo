#pragma once

#include <functional>
#include <memory>

#include "noncopyable.h"
#include "Timestamp.h"

/**
 * 理清楚  EventLoop、Channel、Poller之间的关系 --- Reactor模型上对应 Demultiplex
 * Channel 理解为通道，封装了 [sockfd] 和其感兴趣的 [event]，如EPOLLIN、EPOLLOUT事件
 * 还绑定了poller返回的具体事件
 */

class EventLoop;

class Channel : noncopyable
{
public:
    using EventCallback = std::function<void()>;
    using ReadEventCallback = std::function<void(Timestamp)>;
    
    Channel(EventLoop *loop, int fd);
    ~Channel();

    // 处理事件 --- fd得到poller通知以后，处理事件的（调用相应的回调方法）
    void handleEvent(Timestamp receiveTime);

    // 设置回调函数对象
    void setReadCallback(ReadEventCallback cb) { readEventCallback_ = std::move(cb); }
    void setWriteCallback(EventCallback cb) { writeCallback_ = std::move(cb); }
    void setCloseCallback(EventCallback cb) { closeCallback_ = std::move(cb); }
    void setErrorCallback(EventCallback cb) { errorCallback_ = std::move(cb); }

    // 防止当 channel 被手动 remove 掉，channel 还在执行回调操作（所以 channel 用了一个弱智能指针 std::weak_ptr<void> tie_; 来监听当前 channel 是否已经被 remove 掉）
    void tie(const std::shared_ptr<void>&);

    int fd() const { return fd_; }
    int events() const { return events_; }
    void set_revents(int revt) { revents_ = revt; } // poller 监听事件，设置 channel 所表达的这个 fd 相应的发生的事件

    // 使能操作 --- 设置 fd 相应的事件状态
    void enableReading() { events_ |= kReadEvent; update(); }   // 相应位 置 ReadEvent
    void disableReading() { events_ &= ~kReadEvent; update(); } // 相应位 去掉
    void enableWriting() { events_ |= kWriteEvent; update(); }
    void disableWriting() { events_ &= ~kWriteEvent; update();}
    void disableAll() { events_ = kNoneEvent; update(); }

    // 返回fd当前的事件状态
    bool isNoneEvent() const { return events_ == kNoneEvent; }
    bool isWriting() const { return events_ & kWriteEvent; }
    bool isReading() const { return events_ & kReadEvent; }

    int index() { return index_; }
    void set_index(int idx) { index_ = idx; }

    // one loop per thread
    EventLoop* ownerLoop() { return loop_; } // 当前channel属于哪个eventloop（一个线程包含一个eventloop，一个eventloop包含一个poller，一个poller可以监听很多个channel）
    void remove(); // 删除 channel 用的
private:
    void update(); // update就是在调用epoll_ctl
    void handleEventWithGuard(Timestamp receiveTime); // 根据具体接收到的事件来做回调操作

    static const int kNoneEvent;    // 对任何事件都不感兴趣
    static const int kReadEvent;    // 读事件
    static const int kWriteEvent;   // 写事件

    EventLoop * loop_;  // 事件循环 [这个fd属于哪个EventLoop对象]
    const int fd_;      // fd [Poller监听的对象]
    int events_;        // 注册fd感兴趣的事件
    int revents_;       // poller给channel通知的，具体发生的事件 [实际监听到该fd发生的事件类型集合]
    int index_;

    std::weak_ptr<void> tie_;
    bool tied_;

    // Channel通道中能够获知fd最终发生的具体的事件的revents，所以它负责调用具体事件的回调操作
    // 由用户决定，通过接口传给channel，channel负责调用，只有channel知道fd上发生了什么事件
    ReadEventCallback readEventCallback_;
    EventCallback writeCallback_;
    EventCallback closeCallback_;
    EventCallback errorCallback_;
};