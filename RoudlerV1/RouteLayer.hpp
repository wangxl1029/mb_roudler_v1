#pragma once
#include "dispsurf.hpp"

class CRouteMesh : public CMemDevCtxMesh
{
public:
	DECLARE_DYNAMIC(CRouteMesh)
	CRouteMesh(LPRECT pRect, size_t id) : CMemDevCtxMesh(pRect, id)
	{}

	void DrawRoute(nsDispScope::CPolylineManager& rMng, CPen* pRutPen)
	{
		auto& rAry = m_aryDspScpPolylineID;
		CPen* pOldPen = NULL;
		if (pRutPen)
		{
			pOldPen = m_memDc.SelectObject(pRutPen);
		}

		if (rAry.GetSize() > 0)
		{
			TRACE2("DRAW RUT MESH ID#%u, polyline array size is %u\n", m_ID, rAry.GetSize());
		}
		for (INT_PTR plnIdx = 0; plnIdx < rAry.GetSize(); plnIdx++)
		{
			POINT tilePt = DspScpPnt2TilePnt(rMng.GetShapePoint(rAry[plnIdx], 0));
			TRACE3("PLN_ID#%lu {%d, %d}\n", rAry[plnIdx], tilePt.x, tilePt.y);
			m_memDc.MoveTo(tilePt);
			for (INT_PTR iShppt = 1; iShppt < rMng.GetShapePointNum(rAry[plnIdx]); iShppt++)
			{
				tilePt = DspScpPnt2TilePnt(rMng.GetShapePoint(rAry[plnIdx], iShppt));
				TRACE3("PLN_ID#%lu {%d, %d}\n", rAry[plnIdx], tilePt.x, tilePt.y);
				m_memDc.LineTo(tilePt);
			}
		}

		if (pOldPen)
		{
			m_memDc.SelectObject(pOldPen);
		}
	}


	CUIntArray m_aryDspScpPolylineID;
};


class CRouteLayer : public CMeshSurface
{
#define super CMeshSurface
public:
	DECLARE_DYNAMIC(CRouteLayer)
	typedef void* RefHandle_t;
	CRouteLayer() = delete; // not allow default ctor or new
	static RefHandle_t CreateInstance(nsDispScope::Handle_t hDspScp)
	{
		CRouteLayer* pLayer = new CRouteLayer(hDspScp);
		return pLayer->Handle();
	}

	static CRouteLayer* Refer(RefHandle_t h)
	{
		ASSERT_POINTER(h, tagHandleInfo);
		tagHandleInfo* pInfo = static_cast<tagHandleInfo*>(h);
		return pInfo->pMe;
	}

	size_t Release()
	{
		size_t& refcnt = m_HandleInfo.refcnt;
		size_t oldcnt = (refcnt) ? refcnt-- : 0;
		if (oldcnt > 0 && m_bAutoDelete)
		{
			Destroy();
		}

		return oldcnt;
	}

	RefHandle_t Handle() const
	{
		return m_HandleInfo.hMe;
	}

	// helper
	INT_PTR CreatePolyline(nsDispScope::JsonRouteLinkInfo& rLnkinf, LPRECT pDspScpRect);
	BOOL InitRoute();

	// override
	BOOL Initialize(SIZE sizMesh, HDC hDestDC) override
	{
		if (super::Initialize(sizMesh, hDestDC))
		{
			VERIFY(InitRoute());
			DrawMesh();

			return TRUE;
		}

		TRACE0("Route layer initialize failed!\n");
		return FALSE;
	}

	SIZE GetDispScopeAreaSize() final
	{
		return m_pDispScope->GetDispScopeSize();
	}

	POINT GeoPoint2DispScopePoint(JsonGeoPoint pt) final
	{
		return m_pDispScope->GetDispOffset(pt);
	}

	RECT GeoRect2DispScopeRect(JsonGeoPoint p1, JsonGeoPoint p2) final
	{
		return m_pDispScope->GeoRect2DispScopeRect(p1, p2);
	}

	void DrawMesh()
	{
		CPen penRoute;
		penRoute.CreatePen(PS_SOLID, 4, RGB(255, 0, 0));
		for (POSITION pos = m_meshList.GetHeadPosition(); pos != NULL;)
		{
			CRouteMesh* pMesh = DYNAMIC_DOWNCAST(CRouteMesh, m_meshList.GetNext(pos));
			ASSERT(pMesh);
			TRACE1("===> draw route mesh#%u in memdc\n", pMesh->m_ID);
			pMesh->DrawRoute(m_mngPolyline, &penRoute);
		}
	}

	// override
	CMemDevCtxMesh* CreateMesh(LPRECT pRect, size_t id) final
	{
		return new CRouteMesh(pRect, id);
	}
private:
	CRouteLayer(nsDispScope::Handle_t hDispScope) : CMeshSurface()
		, m_bAutoDelete(true)
	{
		ASSERT(hDispScope);
		m_pDispScope = CDisplayScope::Refer(hDispScope);
		ASSERT(m_pDispScope);
		m_dwRasterOperaton = SRCPAINT;

		m_HandleInfo.pMe = this;
		m_HandleInfo.hMe = &m_HandleInfo;
		m_HandleInfo.refcnt = 0;
	}

	~CRouteLayer()
	{
		m_pDispScope->Release();
	}

	void Destroy()
	{
		delete this;
	}
private:
	struct tagHandleInfo
	{
		RefHandle_t hMe;
		CRouteLayer* pMe;
		size_t refcnt;
	}m_HandleInfo;
private:
	const bool m_bAutoDelete;
	CDisplayScope* m_pDispScope;
	nsDispScope::CPolylineManager m_mngPolyline;
#undef super
};

