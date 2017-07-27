#pragma once
#include "myhandle.h"

#ifdef __cplusplus
extern "C"{
#endif
	typedef struct MyMessage_tag{
		size_t mID;
		void* mData;
	}MyMessage_t;

	typedef enum{
		MY_WAIT_MSG_STATE_OK = 0,
		MY_WAIT_MSG_STATE_TIMEOUT,
		MY_WAIT_MSG_STATE_NO_MESSAGE,
		MY_WAIT_MSG_STATE_INVALID,
		MY_WAIT_MSG_STATE_ERR_UNKNOWN,
		MY_WAIT_MSG_STATE_
	}MyWaitMsgState_E;

	typedef void(*MyThreadProc_t)(void*);

	MyHandle_t MyThread_create(MyThreadProc_t pfn, void* param);
	bool MyThread_destroy(MyHandle_t t);

	void MyThread_join(MyHandle_t t);
	MyWaitMsgState_E MyThread_waitMessage(MyHandle_t t, MyMessage_t* m, size_t tm_out);
	void MyThread_sendMessage(MyHandle_t h, MyMessage_t* m);
	void MyThread_sleep_ms(size_t n);
#ifdef __cplusplus
}
#endif
