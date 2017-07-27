#include "stdafx.h"
#include "SrchBrchLayer.hpp"


IMPLEMENT_DYNAMIC(CRutSrchBrchMesh, CMemDevCtxMesh);
IMPLEMENT_DYNAMIC(CSearchBranchLayer, CMeshSurface); 

void CRutSrchBrchMesh::DrawSearchBranch(CBrush* pBgBr, CPen* pFwdPen, CPen* pRevPen, CPen* pRutPen)
{
	ASSERT(m_memDc.m_hDC);
	if (pBgBr)
	{
		CBrush* pOldBr = m_memDc.SelectObject(pBgBr);
		CRect rctTileBg(0, 0, GetDispScopeRect().Width(), GetDispScopeRect().Height());
		VERIFY(m_memDc.Rectangle(rctTileBg));
		m_memDc.SelectObject(pOldBr);
	}

	auto& rl = m_DSegDispInfoList;
	for (POSITION pos = rl.GetHeadPosition(); pos != NULL;)
	{
		CPen* pOldPen = NULL;
		auto& di = *rl.GetNext(pos);
		if (di.isRoute && pRutPen)
		{
			pOldPen = m_memDc.SelectObject(pRutPen);
		}
		else if (di.isRev && pRevPen)
		{
			pOldPen = m_memDc.SelectObject(pRevPen);
		}
		else if (!di.isRev && pFwdPen)
		{
			pOldPen = m_memDc.SelectObject(pFwdPen);
		}

		m_memDc.MoveTo(DspScpPnt2TilePnt(di.ptFirst));
		m_memDc.LineTo(DspScpPnt2TilePnt(di.ptLast));

		if (pOldPen)
		{
			m_memDc.SelectObject(pOldPen);
		}
	}
}




//////////////////////////////////////////////////////////////////////////////////////////
// CSearchBranchLayer
//////////////////////////////////////////////////////////////////////////////////////////
CSearchBranchLayer::CSearchBranchLayer(nsDispScope::Handle_t hDispScope) : CMeshSurface(), m_bAutoDelete(true)
{
	ASSERT(hDispScope);
	m_pDispScope = CDisplayScope::Refer(hDispScope);
	ASSERT(m_pDispScope);

	m_HandleInfo.hMe = &m_HandleInfo;
	m_HandleInfo.pMe = this;
	m_HandleInfo.refcnt = 0;
}

CSearchBranchLayer::RefHandle_t CSearchBranchLayer::CreateInstance(nsDispScope::Handle_t hDspScp)
{
	CSearchBranchLayer* pLayer = new CSearchBranchLayer(hDspScp);
	return pLayer->m_HandleInfo.hMe;
}

CSearchBranchLayer* CSearchBranchLayer::Refer(RefHandle_t hAny)
{
	ASSERT_POINTER(hAny, tagHandleInfo);
	tagHandleInfo* pInfo = static_cast<tagHandleInfo*>(hAny);
	pInfo->refcnt++;

	return pInfo->pMe;
}

size_t CSearchBranchLayer::Release()
{
	auto& rCnt = m_HandleInfo.refcnt;
	size_t oldcnt = (rCnt > 0) ? rCnt-- : 0;
	if (0 == oldcnt && m_bAutoDelete)
		Destroy();

	return oldcnt;
}

void CSearchBranchLayer::InitRouteLinkCache()
{
	auto &mp = m_mapDsid2RutLnkIdx;
	if (!mp.IsEmpty())
	{
		mp.RemoveAll();
	}

	JsonRouteInfo ri;
	JsonRouteLink rlnk;
	size_t routenum = Json_getRouteNum();
	TRACE1("route number is %d.\n", routenum);
	for (size_t i = 0; i < routenum; ++i)
	{
		bool ri_ok = Json_getRouteInfo(i, &ri);
		for (size_t j = 0; j < ri.mLinkNum && Json_getRouteLink(i, j, &rlnk); j++)
		{
			// link cache
			RouteLinkIndex_stc val;
			val.mIdx[0] = val.mIdx[1] = val.mIdx[2] = INT_MAX;
			mp.Lookup(rlnk.mDsid, val);
			val.mIdx[i] = j;
			mp.SetAt(rlnk.mDsid, val);

			TRACE3("link[%d][%d] disd %llu\n", i, j, rlnk.mDsid);

			// access all the shape point of the current link
			JsonGeoPoint geoShpPnt;
			//::PostMessage(m_hWndTarget, WM_MYNOTIFY, E_TID_DISP_PREP_RUTLNK_SHPPNT_NUM, rlnk.mShpPntNum);
			for (size_t k = 0; k < rlnk.mShpPntNum && Json_getRouteLinkShapePoint(i, j, k, &geoShpPnt); k++)
			{

			}
		}

		if (ri_ok)
		{
			TRACE2("link num %d, cost %d\n", ri.mLinkNum, ri.mCost);
		}
	}

	//TRACE1("===> the nubmer of route link in cache is %u", m_mapDsid2RutLnkIdx.GetCount());
	//::PostMessage(m_hWndTarget, WM_MYNOTIFY, E_TID_DISP_PREP_RUTLNK_CACHED, m_mapDsid2RutLnkIdx.GetCount());
}

BOOL CSearchBranchLayer::InitSearchBranch()
{
	CRect dispScpMeshPort;
	JsonFromInfo fi;
	bool bInitDefPos = false;
	auto br_num = Json_getRouteSrchBranchNum();
	for (size_t i = 0; i < br_num; i++)
	{
		bool fi_ok = Json_getSrchBrchFromInfo(i, &fi);
		//TRACE2("===> branch[%d] out seg number %d\n", i, fi.m_outnum);
		if (fi_ok)
		{
			CRect dsegDspScpRect = GeoRect2DispScopeRect(fi.m_firstShpPt, fi.m_lastShpPt);
			for (POSITION pos = m_meshList.GetHeadPosition(); pos != NULL;)
			{
				CRutSrchBrchMesh* pCurMsh = DYNAMIC_DOWNCAST(CRutSrchBrchMesh, m_meshList.GetNext(pos));
				ASSERT(pCurMsh);
				if (dispScpMeshPort.IntersectRect(dsegDspScpRect, pCurMsh->GetDispScopeRect()))
				{
					tagDSegDispInfo* pDI = new tagDSegDispInfo;
					auto& rmp = m_mapDsid2RutLnkIdx;
					RouteLinkIndex_stc rutidx;

					pDI->isRev = fi.m_isRevSrh;
					pDI->isRoute = !!rmp.Lookup(fi.m_dsid, rutidx); // BOOL to bool
					pDI->ptFirst = GeoPoint2DispScopePoint(fi.m_firstShpPt);
					pDI->ptLast = GeoPoint2DispScopePoint(fi.m_lastShpPt);

					//TRACE1("====> fi.m_dsid %d \n", fi.m_dsid);
					pCurMsh->AddDSegDispInfo(pDI);

					if (!bInitDefPos)
					{
						m_pntTopLeft = pDI->ptFirst;
						//m_pntTopLeft.Offset(-500, -500);

						bInitDefPos = true;
					}
				}
			}
		}
	}

	return (br_num > 0);
}

void CSearchBranchLayer::DrawMesh()
{
	CPen penOrder, penRev, penRoute;
	penOrder.CreatePen(PS_SOLID, 2, RGB(0, 255, 255));
	penRev.CreatePen(PS_SOLID, 2, RGB(0, 0, 255));
	penRoute.CreatePen(PS_SOLID, 2, RGB(0, 255, 0));
	CBrush brBg;
	brBg.CreateSolidBrush(RGB(128, 128, 128));
	for (POSITION pos = m_meshList.GetHeadPosition(); pos != NULL;)
	{
		CRutSrchBrchMesh* pMesh = DYNAMIC_DOWNCAST(CRutSrchBrchMesh, m_meshList.GetNext(pos));
		ASSERT(pMesh);
		//TRACE1("===> draw mesh#%u in memdc\n", pMesh->m_ID);
		pMesh->DrawSearchBranch(&brBg, &penOrder, &penRev, &penRoute);
	}
}
