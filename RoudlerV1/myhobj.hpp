#pragma once

namespace CPP11_myhandle
{

	struct CHandleObject
	{
		virtual ~CHandleObject() {}
		virtual void closeObject() = 0;
	};
}

