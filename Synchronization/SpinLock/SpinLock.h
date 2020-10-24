#include<atomic>

using std::atomic;

//自旋锁
class SpinLock
{
public:
    SpinLock()
        : _flag(false)
    {}
    
    //防拷贝
    SpinLock(const SpinLock&) = delete;
    SpinLock& operator=(const SpinLock&) = delete;

    void lock()
    {
        bool expect = false;    //旧预期值

        //采用CAS,当旧预期值相符的时候将其替换为新预期值
        while(!_flag.compare_exchange_weak(expect, true))
        {
            //由于weak可能会执行失败，并且旧预期值会被修改，所以需要复原旧预期值
            expect = false;
        }
    }

    void unlock()
    {
        _flag.store(false); //将值替换回false即可
    }

private:
    atomic<bool> _flag;
};

//利用RAII机制封装的自旋锁的守卫锁
class spin_lock_guard
{
public:
    spin_lock_guard(SpinLock& spLock)
        : _spLock(spLock)
    {
        _spLock.lock();
    }

    ~spin_lock_guard()
    {
        _spLock.unlock();
    }

    //防拷贝
    spin_lock_guard(const spin_lock_guard&) = delete;
    spin_lock_guard& operator=(const spin_lock_guard&) = delete;
private:
    SpinLock& _spLock;
};