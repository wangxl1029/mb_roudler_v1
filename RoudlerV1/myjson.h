#pragma once
#include "myhandle.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"{
#endif
	typedef void(*JsonRouteSrchDataCompleteCallbackFn_t)(MyHandle_t);
	typedef uint32_t RouteCost_t;
	typedef uint32_t NavInfoLinkId_t;
	typedef uint64_t DSegmentId_t;

	struct JsonGeoPoint
	{
		uint32_t x, y;
	};

	struct JsonRouteSrchGeoInfo
	{
		JsonGeoPoint mMaxFromRectTopLeft;
		JsonGeoPoint mMaxFromRectBtmRight;
	};

	struct JsonFromInfo
	{
		bool m_isRevSrh;
		bool m_isSuper;
		DSegmentId_t m_dsid;
		NavInfoLinkId_t m_nilid;
		JsonGeoPoint m_firstShpPt;
		JsonGeoPoint m_lastShpPt;
		size_t m_outnum;
	};

	struct JsonOutInfo
	{
		bool m_isSuper;
		DSegmentId_t m_dsid;
		NavInfoLinkId_t m_nilid;
		RouteCost_t m_cost;
	};

	struct JsonRouteInfo
	{
		size_t mLinkNum;
		RouteCost_t mCost;
		size_t mLampNum;
	};

	struct JsonRouteLink
	{
		DSegmentId_t mDsid;
		size_t mShpPntNum;
	};

	void Json_setRouteSearchDataCallBack(JsonRouteSrchDataCompleteCallbackFn_t);
	bool Json_asynLoadRouteSearchData(const char*);
	size_t Json_getRouteSrchBranchNum();
	bool Json_getRouteSrchGeoInfo(JsonRouteSrchGeoInfo*);
	bool Json_getSrchBrchFromInfo(size_t, JsonFromInfo*);
	bool Json_getBranchOutInfo(size_t, size_t, JsonOutInfo*);
	size_t Json_getRouteNum();
	bool Json_getRouteInfo(size_t rutidx, JsonRouteInfo*);
	bool Json_getRouteLink(size_t rutidx, size_t lnkidx, JsonRouteLink*);
	bool Json_getRouteLinkShapePoint(size_t rutidx, size_t lnkidx, size_t pntidx, JsonGeoPoint* pPt);
	void Json_finalize();

#ifdef __cplusplus
}
#endif
