#ifndef LOCKER_H
#define LOCKER_H

#include <exception>
#include <pthread.h>
#include <semaphore.h>

// 类中主要是Linux下三种锁进行封装，将锁的创建于销毁函数封装在类的构造与析构函数中，实现RAII机制

class sem
{
public:
    sem()
    {
        // 初始化一个为命名的信号量
        // 参数：1.信号量的指针 2.共享方式。0：线程间共享；1：进程间共享，需要共享内存 3.信号量初始值
        if (sem_init(&m_sem, 0, 0) != 0)
        {
            throw std::exception();
        }
    }
    // sem的有参构造
    sem(int num)
    {
        if (sem_init(&m_sem, 0, num) != 0)
        {
            throw std::exception();
        }
    }
    ~sem()
    {
        // 用于销毁信号量，释放其占用的内核资源
        sem_destroy(&m_sem);
    }
    bool wait()
    {
        // 以原子操作的方式将信号量的值减一，如果信号量的值为0，则sem_wait阻塞
        // sem_wait()成功返回0,失败返回-1。
        return sem_wait(&m_sem) == 0;
    }
    bool post()
    {
        // 以原子操作的方式将信号量的值加1，当信号量的值大于0时，其他正在调用sem_wait等待信号量的线程将被唤醒
        return sem_post(&m_sem) == 0;
    }

private:
    sem_t m_sem;
};

class locker
{
public:
    locker()
    {
        // 初始化互斥锁
        // 参数: 第二个参数表示互斥锁的属性，NULL表示默认属性
        if (pthread_mutex_init(&m_mutex, NULL) != 0)
        {
            throw std::exception();
        }
    }
    ~locker()
    {
        // 销毁锁
        pthread_mutex_destroy(&m_mutex);
    }
    bool lock()
    {
        // 以原子操作的方式给一个互斥锁上锁
        return pthread_mutex_lock(&m_mutex) == 0;
    }
    bool unlock()
    {
        // 以原子操作的方式给一个互斥锁解锁
        return pthread_mutex_unlock(&m_mutex) == 0;
    }
    // 获取锁 
    pthread_mutex_t *get()
    {
        return &m_mutex;
    }

private:
    // 互斥锁的类型是pthread_mutex_t结构体
    pthread_mutex_t m_mutex;
};

class cond
{
public:
    cond()
    {
        // 初始化条件变量 第二个参数是设置属性
        if (pthread_cond_init(&m_cond, NULL) != 0)
        {
            //pthread_mutex_destroy(&m_mutex);
            throw std::exception();
        }
    }
    ~cond()
    {
        // 销毁条件变量
        pthread_cond_destroy(&m_cond);
    }
    // 需要传入一个mutex
    bool wait(pthread_mutex_t *m_mutex)
    {
        int ret = 0;
        //pthread_mutex_lock(&m_mutex);
        // 等待目标条件变量
        ret = pthread_cond_wait(&m_cond, m_mutex);
        //pthread_mutex_unlock(&m_mutex);
        return ret == 0;
    }
    bool timewait(pthread_mutex_t *m_mutex, struct timespec t)
    {
        int ret = 0;
        //pthread_mutex_lock(&m_mutex);
        ret = pthread_cond_timedwait(&m_cond, m_mutex, &t);
        //pthread_mutex_unlock(&m_mutex);
        return ret == 0;
    }
    bool signal()
    {
        // 唤醒一个等待条件变量的线程
        return pthread_cond_signal(&m_cond) == 0;
    }
    bool broadcast()
    {
        // 以广播的形式唤醒所有等待条件变量的线程
        return pthread_cond_broadcast(&m_cond) == 0;
    }

private:
    //static pthread_mutex_t m_mutex;
    pthread_cond_t m_cond;
};
#endif
