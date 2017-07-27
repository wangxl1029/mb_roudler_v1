#include "stdafx.h"
#include "RouteLayer.hpp"

IMPLEMENT_DYNAMIC(CRouteMesh, CMemDevCtxMesh);
IMPLEMENT_DYNAMIC(CRouteLayer, CMeshSurface);

BOOL CRouteLayer::InitRoute()
{
	CRect dispScpMeshPort;
	nsDispScope::JsonRouteLinkInfo lnkinf;
	const size_t rutnum = Json_getRouteNum();
	for (size_t rutidx = 0; rutidx < rutnum && Json_getRouteInfo(rutidx, &lnkinf.mRutInf); rutidx++)
	{
		lnkinf.mRutidx = rutidx;
		for (size_t lnkidx = 0; lnkidx < lnkinf.mRutInf.mLinkNum && Json_getRouteLink(rutidx, lnkidx, &lnkinf.mRutlnk); lnkidx++)
		{
			lnkinf.mLnkidx = lnkidx;
			RECT rctMax;
			auto polyline_id = CreatePolyline(lnkinf, &rctMax);
			TRACE2("link#%u polyline id %u\n", lnkidx, polyline_id);
			//TRACE2("top %d left %d\n", rctMax.top, rctMax.left);
			//TRACE2("btm %d right %d\n", rctMax.bottom, rctMax.right);
			for (POSITION pos = m_meshList.GetHeadPosition(); pos != NULL;)
			{
				CRouteMesh* pCurMsh = DYNAMIC_DOWNCAST(CRouteMesh, m_meshList.GetNext(pos));
				ASSERT(pCurMsh);
				if (dispScpMeshPort.IntersectRect(&rctMax, pCurMsh->GetDispScopeRect()))
				{
					TRACE2("mesh#%u add polyline %lu\n", pCurMsh->m_ID, polyline_id);
					pCurMsh->m_aryDspScpPolylineID.Add(polyline_id);
				}
			}
		}
	}

	return (rutnum > 0);
}

INT_PTR CRouteLayer::CreatePolyline(nsDispScope::JsonRouteLinkInfo& rLnkinf, LPRECT pDspScpRect = NULL)
{
	CArray<POINT>* pPln = new CArray<POINT>;
	pPln->SetSize(rLnkinf.mRutlnk.mShpPntNum);

	JsonGeoPoint preGeoPnt, curGeoPnt;
	for (size_t pntidx = 0; pntidx < rLnkinf.mRutlnk.mShpPntNum && Json_getRouteLinkShapePoint(rLnkinf.mRutidx, rLnkinf.mLnkidx, pntidx, &curGeoPnt); pntidx++)
	{
		//CRect shpLnkDspScpRect = GeoRect2DispScopeRect(preGeoPnt, curGeoPnt);
		pPln->SetAt(pntidx, GeoPoint2DispScopePoint(curGeoPnt));
		if (pDspScpRect)
		{
			if (pntidx > 0)
			{
				CRect rctCur(GeoRect2DispScopeRect(preGeoPnt, curGeoPnt));
				if (pntidx > 1)
				{
					CRect rctUnion, rctLast(pDspScpRect);
					rctUnion.UnionRect(&rctLast, &rctCur);
					*pDspScpRect = rctUnion;
				}
				else
				{
					*pDspScpRect = rctCur;
				}
			}

			preGeoPnt = curGeoPnt;
		}
	}

	return m_mngPolyline.Add(pPln);
}
