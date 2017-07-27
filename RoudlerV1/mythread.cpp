#include <queue>
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>

#include "mythread.h"

#define MY_ASSERT
#define MY_LOG_WARN
#define MY_THREAD_CHECK(_TH_)

namespace CPP_11
{
	class MyMessageQueue
	{
	public:
		MyMessageQueue() = default;

	public:
		bool wait_and_pop(MyMessage_t& m, size_t tm_out_ms)
		{
			bool isTimeout = false;
			std::unique_lock<std::mutex> lk(m_mutex);

			if (m_cv.wait_for(lk, std::chrono::milliseconds(tm_out_ms), [this]{return !m_mq.empty(); }))
			{
				m = m_mq.front();
				m_mq.pop();
			}
			else
			{
				isTimeout = true;
			}

			return (!isTimeout);
		}

		void push(const MyMessage_t& m)
		{
			std::lock_guard<std::mutex> lk(m_mutex);
			m_mq.push(m);
			m_cv.notify_one();
			return;
		}
	private:
		std::queue<MyMessage_t> m_mq;
		std::mutex m_mutex;
		std::condition_variable m_cv;
	};

	class MyThread
	{
	public: ///< lifecycle
		MyThread() = delete;
		MyThread(const MyThread&) = delete;
		MyThread(std::thread* t)
			: m_thread(t)
			, m_mq(nullptr)
		{
			MY_ASSERT(t);
		}

		~MyThread()
		{
			if (m_thread)
			{
				MY_ASSERT(!m_thread->joinable());
				delete m_thread;
				m_thread = nullptr;
			}

			if (m_mq)
			{
				delete m_mq;
				m_mq = nullptr;
			}
		}
	public:
		void join()
		{
			if (m_thread)
			{
				if (m_thread->joinable())
				{
					m_thread->join();
				}
			}

			return;
		}

		MyWaitMsgState_E wait_msg(MyMessage_t& m, size_t tm_out_ms)
		{
			MyWaitMsgState_E retval = MY_WAIT_MSG_STATE_OK;

			std::call_once(m_flag_mq, &MyThread::createQueue, this);
			if (m_mq)
			{
				//MY_LOG_INFO("wait_and_pop...");
				if (m_mq->wait_and_pop(m, tm_out_ms))
				{

				}
				else
				{
					retval = MY_WAIT_MSG_STATE_TIMEOUT;
				}
			}
			else
			{
				retval = MY_WAIT_MSG_STATE_INVALID;
			}

			return retval;
		}

		void send_msg(const MyMessage_t& m)
		{
			std::call_once(m_flag_mq, &MyThread::createQueue, this);
			if (m_mq)
			{
				m_mq->push(m);
				//MY_LOG_INFO("sent msgid#%d", m.m_ID);
			}

			return;
		}
	private:
		void createQueue()
		{
			m_mq = new MyMessageQueue;
		}
	private:
		std::thread* m_thread;
		std::once_flag m_flag_mq;
		MyMessageQueue* m_mq;
	};

}

using namespace CPP_11;

#ifdef __cplusplus
extern "C"{
#endif

MyHandle_t MyThread_create(MyThreadProc_t pfn, void* param)
{
	MyThread* t = new MyThread(new std::thread(pfn, param));

	return static_cast<MyHandle_t>(t);
}

bool MyThread_destroy(MyHandle_t h)
{
	if (IS_MY_VALID_HANDLE(h))
	{
		MY_THREAD_CHECK(h);

		MyThread* t = static_cast<MyThread*>(h);

		delete t;
	}

	return true;
}


void MyThread_join(MyHandle_t h)
{
	MY_ASSERT(IS_MY_VALID_HANDLE(h));
	MY_THREAD_CHECK(h);

	MyThread* t = static_cast<MyThread*>(h);
	t->join();

	return;
}

MyWaitMsgState_E MyThread_waitMessage(MyHandle_t h, MyMessage_t* m, size_t tm_out)
{
	MY_ASSERT(IS_MY_VALID_HANDLE(h));
	MY_ASSERT(m);

	MyThread* t = static_cast<MyThread*>(h);

	return t->wait_msg(*m, tm_out);
}

void MyThread_sendMessage(MyHandle_t h, MyMessage_t* m)
{
	MY_ASSERT(IS_MY_VALID_HANDLE(h));

	if (m)
	{
		MyThread* t = static_cast<MyThread*>(h);
		t->send_msg(*m);
	}
	else
	{
		MY_LOG_WARN("NULL message pointer!");
	}
	
	return;
}

void MyThread_sleep_ms(size_t n)
{
	std::this_thread::sleep_for(std::chrono::microseconds(n));
}

#ifdef __cplusplus
}
#endif