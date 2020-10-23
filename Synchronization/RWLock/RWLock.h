#include<memory>
#include<mutex>
#include<condition_variable>

using std::mutex;
using std::condition_variable;
using std::unique_lock;

//读写锁
class RWLock
{
public:
        RWLock()
            : _isWriting(false)
            , _readCount(0)
            , _writeCount(0)
        {}
        
        //防拷贝
        RWLock(const RWLock&) = delete;
        RWLock& operator=(const RWLock&) = delete;

        void read_lock();
        void read_unlock();
        void write_lock();
        void write_unlock();

private:
    volatile bool _isWriting;
    volatile int _readCount;
    volatile int _writeCount;
    mutex _mtx;
    condition_variable _readCond;
    condition_variable _writeCond;
};

void RWLock::read_lock()
{
    unique_lock<mutex> lock(_mtx);

    //如果当前没人在写，并且也没有写锁等待时可读
    _readCond.wait(lock, [this]()->bool{
        return _writeCount == 0;
    });

    //计数增加
    ++_readCount;
}

void RWLock::read_unlock()
{
    unique_lock<mutex> lock(_mtx);
    --_readCount;
    
    //如果当前读者为0，并且有正在等待的写者，则唤醒一个写者。
    if(_readCount == 0 && _writeCount > 0)
    {
        _writeCond.notify_one();
    }
}

void RWLock::write_lock()
{
    unique_lock<mutex> lock(_mtx);
    ++_writeCount;  //先增加计数，保证写者优先

    //如果当前没人在写也没人读，则说明可写
    _readCond.wait(lock, [this]()->bool{
        return _readCount == 0 && _isWriting == false;
    });

    _isWriting = true;
}

void RWLock::write_unlock()
{
    unique_lock<mutex> lock(_mtx);
    --_writeCount;
    _isWriting = false;

    //如果当前没有其他的写锁在等待，则唤醒全部读锁
    if(_writeCount == 0)
    {
        _readCond.notify_all();
    }
    //如果当前还有别的写锁等待，则唤醒该写锁
    else
    {   
        _writeCond.notify_one();
    }
}

//用RAII机制封装读写锁

//读锁守卫锁
class read_lock_guard
{
public:
    read_lock_guard(RWLock& read_lock)
        : _read_lock(read_lock)
    {
        _read_lock.read_lock();
    }

    ~read_lock_guard()
    {
        _read_lock.read_unlock();
    }

    //防拷贝
    read_lock_guard(const read_lock_guard&) = delete;
    read_lock_guard& operator=(const read_lock_guard&) = delete;
private:
    RWLock& _read_lock;
};

//写锁守卫锁
class write_lock_guard
{
public:
    write_lock_guard(RWLock& write_lock)
        : _write_lock(write_lock)
    {
        _write_lock.write_lock();
    }

    ~write_lock_guard()
    {
        _write_lock.write_unlock();
    }

    //防拷贝
    write_lock_guard(const write_lock_guard&) = delete;
    write_lock_guard& operator=(const write_lock_guard&) = delete;
private:
    RWLock& _write_lock;
};