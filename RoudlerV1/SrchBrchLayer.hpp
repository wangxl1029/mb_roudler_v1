#pragma once
#include "dispsurf.hpp"

struct tagDSegDispInfo
{
	bool isRev;
	bool isRoute;
	POINT ptFirst, ptLast; // the position in the display scope
	size_t mShpPntNum;
	POINT* aryShpPnt;
};

class CRutSrchBrchMesh : public CMemDevCtxMesh
{
public:
	DECLARE_DYNAMIC(CRutSrchBrchMesh)
	CRutSrchBrchMesh(LPRECT pRect, size_t id) : CMemDevCtxMesh(pRect, id)
	{}

	void DrawSearchBranch(CBrush* pBgBr, CPen* pFwdPen, CPen* pRevPen, CPen* pRutPen);

	void AddDSegDispInfo(tagDSegDispInfo* pInfo)
	{
		m_DSegDispInfoList.AddHead(pInfo);
	}
private:
	CTypedPtrList<CPtrList, tagDSegDispInfo*> m_DSegDispInfoList;
};

class CSearchBranchLayer : public CMeshSurface
{
#define super CMeshSurface
public:
	DECLARE_DYNAMIC(CSearchBranchLayer)
	typedef void* RefHandle_t;
	CSearchBranchLayer() = delete; // default ctro not allowed
	static RefHandle_t CreateInstance(nsDispScope::Handle_t);
	static CSearchBranchLayer* Refer(RefHandle_t); // increase
	RefHandle_t Handle() const
	{
		return m_HandleInfo.hMe;
	}
	size_t Release();	// decrease
	const bool m_bAutoDelete; // invariant

	// helper
	void InitRouteLinkCache();
	BOOL InitSearchBranch();
	void DrawMesh();

	const POINT& GetTopLeft() const
	{
		return m_pntTopLeft;
	}

	// override
	BOOL Initialize(SIZE sizMesh, HDC hDestDC) override
	{
		if (super::Initialize(sizMesh, hDestDC))
		{
			InitRouteLinkCache();

			VERIFY(InitSearchBranch());
			DrawMesh();
			return TRUE;
		}

		TRACE0("Search branch initialize failed!\n");
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
private:
	CSearchBranchLayer(nsDispScope::Handle_t hDispScope);
	~CSearchBranchLayer()
	{
		m_pDispScope->Release();
	}
	void Destroy() { delete this; }

	CMemDevCtxMesh* CreateMesh(LPRECT pRect, size_t id)
	{
		return new CRutSrchBrchMesh(pRect, id);
	}

private: // handle info
	struct tagHandleInfo
	{
		size_t refcnt;
		RefHandle_t hMe;
		CSearchBranchLayer* pMe;
	}m_HandleInfo;

private:
	POINT m_pntTopLeft;
	CDisplayScope* m_pDispScope;
	struct RouteLinkIndex_stc
	{
		int mIdx[3];
	};
	CMap<DSegmentId_t, DSegmentId_t, RouteLinkIndex_stc, RouteLinkIndex_stc> m_mapDsid2RutLnkIdx;
#undef super
};


