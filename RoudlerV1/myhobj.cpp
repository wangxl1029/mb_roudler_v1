#include "myhobj.hpp"
#include "myhandle.h"

#ifdef __cplusplus
extern "C"{
#endif

	void My_closeHande(MyHandle_t h)
	{
		if (h)
		{
			auto o = static_cast<CPP11_myhandle::CHandleObject*>(h);
			o->closeObject();
		}
	}
#ifdef __cplusplus
}
#endif



