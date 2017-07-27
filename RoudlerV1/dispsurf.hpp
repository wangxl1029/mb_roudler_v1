#pragma once
#include "myjson.h"

namespace nsDispScope
{
	struct JsonRouteLinkInfo
	{
		size_t mRutidx, mLnkidx;
		JsonRouteInfo mRutInf;
		JsonRouteLink mRutlnk;
	};


	class CPolylineManager : public CObject
	{
	public:
		DECLARE_DYNAMIC(CPolylineManager)

		INT_PTR Add(CArray<POINT>* pPln)
		{
			return m_aryDspScpPolyline.Add(pPln);
		}

		void Cleanup()
		{
			auto& ary = m_aryDspScpPolyline;
			for (INT_PTR i = 0; i < ary.GetCount(); i++)
			{
				delete ary[i];
			}
			ary.RemoveAll();
		}

		INT_PTR GetShapePointNum(INT_PTR id)
		{
			CArray<POINT>* pPln = dynamic_cast<CArray<POINT>*>(m_aryDspScpPolyline[id]);
			return pPln->GetCount();
		}

		POINT GetShapePoint(INT_PTR id, INT_PTR shpptIdx)
		{
			CArray<POINT>* pPln = dynamic_cast<CArray<POINT>*>(m_aryDspScpPolyline[id]);
			return pPln->GetAt(shpptIdx);
		}

		LPPOINT GetShapePointData(INT_PTR id)
		{
			CArray<POINT>* pPln = dynamic_cast<CArray<POINT>*>(m_aryDspScpPolyline[id]);
			return pPln->GetData();
		}
	private:
		CObArray m_aryDspScpPolyline;
	};

	typedef void* Handle_t;

}

struct CDispRect4JsonGeo
{
	CDispRect4JsonGeo(JsonGeoPoint ptTL, JsonGeoPoint ptBR)
	: mTopLeft(ptTL), mBtmRight(ptBR)
	{
		Normalize();
	}

	void Swap(uint32_t& a, uint32_t& b)
	{
		uint32_t c = a; a = b; b = c;
	}

	bool IsNormalRectX() const
	{
		return mTopLeft.x < mBtmRight.x;
	}

	bool IsNormalRectY() const
	{
		return mTopLeft.y > mBtmRight.y;
	}

	bool IsNormalRect()
	{
		return IsNormalRectX() && IsNormalRectY();
	}

	void Normalize()
	{
		if (!IsNormalRectX()) Swap(mTopLeft.x, mBtmRight.x);
		if (!IsNormalRectY()) Swap(mTopLeft.y, mBtmRight.y);
	}

	size_t Width() const
	{
		return mBtmRight.x - mTopLeft.x;
	}

	size_t Height() const
	{
		return mTopLeft.y - mBtmRight.y;
	}

	bool IsRectEmpty() const
	{
		return (Width() == 0 || Height() == 0);
	}

	bool IsRectNull() const
	{
		return (Width() == 0 && Height() == 0);
	}

	JsonGeoPoint GetGeoBtmLeft() const
	{
		JsonGeoPoint pt{ mTopLeft.x, mBtmRight.y };
		return pt;
	}

	POINT GetDispOffset(JsonGeoPoint pt) const
	{
		return POINT{ pt.x - mTopLeft.x, mTopLeft.y - pt.y };
	}

	JsonGeoPoint mTopLeft;
	JsonGeoPoint mBtmRight;
};



class CDisplayScope : public CObject
{
private:
	DECLARE_DYNAMIC(CDisplayScope)
	CDisplayScope() : m_bAutoClose(TRUE), m_nRefCount(0)
	{
		DefInit();
	}

	const SIZE& GetDispScopeSize() const
	{
		return m_sizDispScope;
	}

public:
	CDisplayScope& operator=(const CDisplayScope&) = delete;
	CDisplayScope(const CDisplayScope&) = delete;
	POINT GetDispOffset(const JsonGeoPoint& rPt) const
	{
		CDispRect4JsonGeo rctGeo2disp(m_geoScope.mTopLeft, m_geoScope.mBtmRight);
		return rctGeo2disp.GetDispOffset(rPt);
	}

	RECT GeoRect2DispScopeRect(JsonGeoPoint p1, JsonGeoPoint p2) const
	{
		CDispRect4JsonGeo rctGeo2disp(p1, p2);
		CRect dspRect(GetDispOffset(rctGeo2disp.mTopLeft), SIZE{ (LONG)rctGeo2disp.Width(), (LONG)rctGeo2disp.Height() });
		return dspRect;
	}

	void SetGeoScope(const JsonGeoPoint& rTl, const JsonGeoPoint& rBr)
	{
		m_geoScope.mTopLeft = rTl;
		m_geoScope.mBtmRight = rBr;

		CDispRect4JsonGeo rctMaxFrDisp(rTl, rBr);
		ASSERT(rctMaxFrDisp.IsNormalRect());
		m_sizDispScope.cx = rctMaxFrDisp.Width();
		m_sizDispScope.cy = rctMaxFrDisp.Height();
	}

	INT_PTR CreatePolyline(nsDispScope::JsonRouteLinkInfo& rLnkinf, LPRECT pDspScpRect = NULL)
	{
		CArray<POINT>* pPln = new CArray<POINT>;
		pPln->SetSize(rLnkinf.mRutlnk.mShpPntNum);

		JsonGeoPoint preGeoPnt, curGeoPnt;
		for (size_t pntidx = 0; pntidx < rLnkinf.mRutlnk.mShpPntNum && Json_getRouteLinkShapePoint(rLnkinf.mRutidx, rLnkinf.mLnkidx, pntidx, &curGeoPnt); pntidx++)
		{
			//CRect shpLnkDspScpRect = GeoRect2DispScopeRect(preGeoPnt, curGeoPnt);
			pPln->SetAt(pntidx, GetDispOffset(curGeoPnt));
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
	nsDispScope::CPolylineManager m_mngPolyline;

	static nsDispScope::Handle_t Create()
	{
		return new CDisplayScope;
	}

	static CDisplayScope* Refer(nsDispScope::Handle_t h)
	{
		if (h)
		{
			ASSERT_POINTER(h, CDisplayScope);
			CDisplayScope* ins = static_cast<CDisplayScope*>(h);
			ins->m_nRefCount++;
			return ins;
		}

		return NULL;
	}

	size_t Release()
	{
		size_t oldcnt = (m_nRefCount >0) ? m_nRefCount-- : 0;
		if (0 == m_nRefCount && m_bAutoClose)
		{
			Destroy();
		}

		return oldcnt;
	}

	const BOOL m_bAutoClose;
protected:
	void Destroy()
	{
		delete this;
	}
private:
	void DefInit()
	{
		m_nRefCount = 0;
		m_sizDispScope.cx = m_sizDispScope.cy = 0;
		m_geoScope.mTopLeft = m_geoScope.mBtmRight = { 0, 0 };
	}

	size_t m_nRefCount;
	SIZE m_sizDispScope;
	struct {
		JsonGeoPoint mTopLeft, mBtmRight;
	} m_geoScope;
};

class CMemDevCtxMesh : public CObject
{
public:
	DECLARE_DYNAMIC(CMemDevCtxMesh)
	CMemDevCtxMesh(LPRECT pRect, size_t id)
		: m_rctDisp(pRect)
		, m_pOldBmp(NULL)
		, m_ID(id)
	{}

	~CMemDevCtxMesh()
	{
		if (m_pOldBmp)
		{
			m_memDc.SelectObject(m_pOldBmp);
		}
	}

public: // operator
	BOOL InitMemDC(CDC* pDC)
	{
		if (!m_memDc.CreateCompatibleDC(pDC))
			return FALSE;

		if (!m_bmTile.CreateCompatibleBitmap(pDC, m_rctDisp.Width(), m_rctDisp.Height()))
			return FALSE;

		m_pOldBmp = m_memDc.SelectObject(&m_bmTile);
		return TRUE;
	}

	POINT DspScpPnt2TilePnt(POINT pt) const
	{
		CPoint rtPnt = CPoint(pt) - m_rctDisp.TopLeft();
		return rtPnt;
	}
	const CRect& GetDispScopeRect() const
	{
		return m_rctDisp;
	}

public: // attribue
	CBitmap m_bmTile;
	CBitmap* m_pOldBmp;
	CDC m_memDc;
	const size_t m_ID;
private:
	CRect m_rctDisp; // the rectangle of the mesh in the display scope
};

class CRoudlerSurface : public CObject
{
public:
	DECLARE_DYNAMIC(CRoudlerSurface)
	virtual BOOL Draw(CDC*, LPRECT pRect) = 0;
	virtual CMemDevCtxMesh* CreateMesh(LPRECT pRect, size_t id) = 0;
	virtual ~CRoudlerSurface() {};
};

class CMeshSurface : public CRoudlerSurface
{
public:
	DECLARE_DYNAMIC(CMeshSurface)
	CMeshSurface() : m_dwRasterOperaton(SRCCOPY)
	{}

	virtual BOOL Initialize(SIZE sizMesh, HDC hDestDC)
	{
		SIZE sizDispScope = GetDispScopeAreaSize();
		SIZE sizMeshCnt{ sizDispScope.cx / sizMesh.cx + 1, sizDispScope.cy / sizMesh.cy + 1 };
		const size_t mesh_maxnum = 256;
		const size_t mesh_num = sizMeshCnt.cx * sizMeshCnt.cy;
		if (mesh_num > mesh_maxnum)
		{
			TRACE2("mesh number %lu is over max value %u!\n", mesh_num, mesh_maxnum);
			return FALSE;
		}

		CDC* pDestDC = CDC::FromHandle(hDestDC);
		for (int midy = 0; midy < sizMeshCnt.cy; midy++)
		{
			for (int midx = 0; midx < sizMeshCnt.cx; midx++)
			{
				RECT rec;
				rec.left = midx * sizMesh.cx;
				rec.top = midy * sizMesh.cy;
				rec.right = (midx + 1) * sizMesh.cx;
				rec.bottom = (midy + 1) * sizMesh.cy;
				CMemDevCtxMesh* pMesh = CreateMesh(&rec, midy * sizMeshCnt.cy + midx);
				m_meshList.AddTail(pMesh);

				VERIFY(pMesh->InitMemDC(pDestDC));
			}
		}

		return TRUE;
	};

	virtual SIZE GetDispScopeAreaSize() = 0;
	virtual POINT GeoPoint2DispScopePoint(JsonGeoPoint pt) = 0;
	virtual RECT GeoRect2DispScopeRect(JsonGeoPoint p1, JsonGeoPoint p2) = 0;

	BOOL Draw(CDC* pDC, LPRECT pRect) final
	{
		if (pDC && pRect && !m_meshList.IsEmpty())
		{
			CRect rctMeshPort;
			CRect rctViewPort(pRect);
			//rctViewPort.OffsetRect(m_pntTopLeft);
			BOOL isBackground = FALSE;

			//TRACE("draw something\n");
			for (POSITION pos = m_meshList.GetHeadPosition(); pos != NULL;)
			{
				CMemDevCtxMesh* pMesh = DYNAMIC_DOWNCAST(CMemDevCtxMesh, m_meshList.GetNext(pos));
				ASSERT(pMesh);

				rctMeshPort.IntersectRect(pMesh->GetDispScopeRect(), rctViewPort);
				if (!rctMeshPort.IsRectEmpty())
				{
					//TRACE3("MESH#%d{%d,%d} showed\n", pMesh->m_ID, rctMeshPort.left, rctMeshPort.top);
					CPoint ptMeshTopLeftInView = rctMeshPort.TopLeft() - rctViewPort.TopLeft();
					CPoint tileSrcPnt = pMesh->DspScpPnt2TilePnt(rctMeshPort.TopLeft());
					pDC->BitBlt(ptMeshTopLeftInView.x, ptMeshTopLeftInView.y, rctMeshPort.Width(), rctMeshPort.Height(), &(pMesh->m_memDc), tileSrcPnt.x, tileSrcPnt.y, m_dwRasterOperaton);

					CString strMshID;
					strMshID.Format(_T("ID:%u"), pMesh->m_ID);
					pDC->TextOut(ptMeshTopLeftInView.x, ptMeshTopLeftInView.y, strMshID);
				}
			}
		}

		return TRUE;
	}

	DWORD m_dwRasterOperaton;
	CTypedPtrArray<CObList, CMemDevCtxMesh*> m_meshList;
};

