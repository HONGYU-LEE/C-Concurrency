#include<memory>
#include<mutex>
#include<condition_variable>

using std::mutex;
using std::condition_variable;
using std::unique_lock;

class Semaphore
{
public:
        Semaphore(int count)
            :_count(count)
        {}
        
        //防拷贝
        Semaphore(const Semaphore&) = delete;
        Semaphore& operator=(const Semaphore&) = delete;

        void set(int count)
        {
            _count = count;
        }

        void wait()
        {
            unique_lock<mutex> lock(_mtx);

            if(_count <= 0)
            {
                _cond.wait(lock);
            }
            --_count;
        }

        void signal()
        {
            unique_lock<mutex> lock(_mtx);

            ++_count;
            _cond.notify_one();
        }

private:
    volatile int _count;    //计数器
    mutex _mtx;
    condition_variable _cond;  
};