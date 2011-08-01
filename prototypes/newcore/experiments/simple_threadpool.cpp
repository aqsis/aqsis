// Aqsis
// Copyright (C) 2001, Paul C. Gregory and the other authors and contributors
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
// * Neither the name of the software's owners nor the names of its
//   contributors may be used to endorse or promote products derived from this
//   software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// (This is the New BSD license)

#include <iostream>
#include <vector>
#include <deque>

#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/thread/locks.hpp>

typedef boost::lock_guard<boost::mutex> lock_guard;


static boost::mutex g_coutMutex;
void threadsafe_print(const char* s)
{
    lock_guard l(g_coutMutex);
    std::cout << s;
}


// Really simple thread pool class.
//
// This implementation has a really simple task type, just to get the general
// flavour of how a thread pool object can be coded.
class ThreadPool
{
    public:
        typedef void (*Task)();

    private:
        std::vector<boost::shared_ptr<boost::thread> > m_threads;

        boost::mutex m_tasksMutex;
        std::deque<Task> m_tasks;
        boost::condition_variable m_tasksChangedCondition;
        bool m_threadExit;

        Task popTask()
        {
            boost::unique_lock<boost::mutex> tasksLock(m_tasksMutex);
            while(m_tasks.empty() && !m_threadExit)
                m_tasksChangedCondition.wait(tasksLock);
            if(m_tasks.empty())
                return 0;
            Task t = m_tasks.front();
            m_tasks.pop_front();
            return t;
        }

        class PoolJob
        {
            private:
                ThreadPool* m_thePool;
            public:
                PoolJob(ThreadPool* thePool)
                    : m_thePool(thePool)
                { }

                void operator()()
                {
                    while(Task task = m_thePool->popTask())
                        task();
                }
        };

    public:
        ThreadPool(int nthreads)
            : m_threadExit(false)
        {
            for(int i = 0; i < nthreads; ++i)
                m_threads.push_back(boost::shared_ptr<boost::thread>(new boost::thread(PoolJob(this))));
        }

        ~ThreadPool()
        {
            threadsafe_print("Entering thread pool destructor\n");
            {
                lock_guard tasksLock(m_tasksMutex);
                m_threadExit = true;
            }
            m_tasksChangedCondition.notify_all();
            threadsafe_print("Threads should now exit when all tasks are done.\n");
            for(int i = 0; i < (int)m_threads.size(); ++i)
            {
                m_threads[i]->join();
                threadsafe_print("Thread exit.\n");
            }
        }

        void schedule(Task t)
        {
            {
                lock_guard tasksLock(m_tasksMutex);
                m_tasks.push_back(t);
            }
            m_tasksChangedCondition.notify_one();
        }
};


void hello1()
{
    threadsafe_print("Hello, 1\n");
}
void hello2()
{
    threadsafe_print("Hello, 2\n");
}
void hello3()
{
    threadsafe_print("Hello, 3\n");
}

int main()
{
    ThreadPool pool(4);

    for(int i = 0; i < 10; ++i)
    {
        pool.schedule(&hello1);
        pool.schedule(&hello2);
        pool.schedule(&hello3);
    }

    return 0;
}

