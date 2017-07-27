#pragma once
#define IS_MY_VALID_HANDLE(_HANDLE_) (MY_INVALID_HANDLE != (_HANDLE_))

#ifdef __cplusplus
extern "C"{
#endif

	typedef void* MyHandle_t;
	const MyHandle_t MY_INVALID_HANDLE = ((void*)0);
	void My_closeHande(MyHandle_t);

#ifdef __cplusplus
}
#endif




