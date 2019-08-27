//
// Created by zkk on 2019/05/29.
//
#ifndef SYNC_QUEUE_HPP_
#define SYNC_QUEUE_HPP_
#define LOG_TAG "LOG_TAG_SyncQueue"

#include "Alog_pri.h"
#include <list>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <errno.h>

using namespace std;

template<typename T>
class SyncQueue {
private:
    std::list<T> m_queue;                  //缓冲区
    pthread_mutex_t m_mutex;                    //互斥量和条件变量结合起来使用
    pthread_cond_t m_notEmpty;//不为空的条件变量
    pthread_cond_t m_notFull; //没有满的条件变量
    int m_maxSize;                         //同步队列最大的size

    bool IsFull() const {
        return m_queue.size() == m_maxSize;
    }

    bool IsEmpty() const {
        return m_queue.empty();
    }

public:
    SyncQueue(int maxSize) : m_maxSize(maxSize) {
        pthread_mutex_init(&m_mutex, NULL);
        pthread_cond_init(&m_notEmpty, NULL);
        pthread_cond_init(&m_notFull, NULL);
    }

    ~SyncQueue() {
        clear();
        pthread_mutex_destroy(&m_mutex);
        pthread_cond_destroy(&m_notEmpty);
        pthread_cond_destroy(&m_notFull);
    }
    int PutResult(const T &x) {
        if (IsFull()) {
            //第一次判断如果满了，直接retuan.不加锁
            ALOGI("队列满了 return>>");
            return -1;
        }
        pthread_mutex_lock(&m_mutex);
//        std::lock_guard<std::mutex> locker(m_mutex);

        while (IsFull()) {
//            std::cout << "the blocking queue is full,waiting..." << std::endl;
//            m_notFull.wait(m_mutex);
            //这里一般不可达，极端情况下，就wait .因为在上面已经对isFull 处理。
            pthread_cond_wait(&m_notFull, &m_mutex);
        }
        m_queue.push_back(x);
//        m_notEmpty.notify_one();
        pthread_cond_signal(&m_notEmpty);
        pthread_mutex_unlock(&m_mutex);
        return 0;
    }


    void Put(const T &x) {
        if (IsFull()) {
            //第一次判断如果满了，直接retuan.不加锁
            ALOGI("队列满了 return>>");
            return;
        }
        pthread_mutex_lock(&m_mutex);
//        std::lock_guard<std::mutex> locker(m_mutex);

        while (IsFull()) {
//            std::cout << "the blocking queue is full,waiting..." << std::endl;
//            m_notFull.wait(m_mutex);
            //这里一般不可达，极端情况下，就wait .因为在上面已经对isFull 处理。
            pthread_cond_wait(&m_notFull, &m_mutex);
        }
        m_queue.push_back(x);
//        m_notEmpty.notify_one();
        pthread_cond_signal(&m_notEmpty);
        pthread_mutex_unlock(&m_mutex);
    }

    void Take(T &x) {
//        std::lock_guard<std::mutex> locker(m_mutex);
        pthread_mutex_lock(&m_mutex);
        while (IsEmpty()) {
//            std::cout << "the blocking queue is empty,wating..." << std::endl;
//            m_notEmpty.wait(m_mutex);
            pthread_cond_wait(&m_notEmpty, &m_mutex);
        }

        x = m_queue.front();
        m_queue.pop_front();
        pthread_cond_signal(&m_notFull);
        pthread_mutex_unlock(&m_mutex);
//        m_notFull.notify_one();
    }

    void clear() {
        pthread_mutex_lock(&m_mutex);
        if (!IsEmpty()) {
            m_queue.clear();
        }
        pthread_mutex_unlock(&m_mutex);
    }


public:
};

#endif
