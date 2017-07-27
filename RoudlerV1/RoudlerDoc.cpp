
// RoudlerDoc.cpp : implementation of the CRoudlerDoc class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "Roudler.h"
#endif

#include "RoudlerDoc.h"
#include "InfoView.h"
#include "MainFrm.h"
#include "SrchBrchLayer.hpp"
#include "RouteLayer.hpp"
#include "refobj.hpp"
#include "revtable.hpp"
#include <propkey.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CRoudlerDoc

IMPLEMENT_DYNCREATE(CRoudlerDoc, CDocument)

BEGIN_MESSAGE_MAP(CRoudlerDoc, CDocument)
	ON_COMMAND(ID_FILE_IMPORT_JSON, &CRoudlerDoc::OnFileImportJson)
END_MESSAGE_MAP()
extern HWND g_hViewWnd;

LPTSTR g_szDocNick = _T("roudler");
#define INFO_SEPERATOR_LINE _T("\t--------8<-------\r\n")

enum class WORK_CMD_E : size_t
{
	NONE,
	REQ_PREPARE_DISP,
	ACQ_PREPARE_DISP,
	MAX
};
typedef CTypedPtrList<CObList, CRoudlerSurface*> SurfaceList_t;

static size_t CvtToCmd(WORK_CMD_E eCmd)
{
	return static_cast<size_t>(eCmd);
}

static WORK_CMD_E CvtToCmd(size_t cmd)
{
	return static_cast<WORK_CMD_E>(cmd);
}

enum class UserCmdType_E : size_t
{
	UNKNOWN,
	PRINT_JSON_INFO,
	PRINT_DSID_FROM_INFO,
	PRINT_DSID_ALL_PARENT_COST,
	MAX
};

class CUserCommand
{
public:
	enum ValueType_E{VT_UNKNOWN, VT_DSID, VT_MAX};

	struct CValueBase
	{
		ValueType_E mValType;
		virtual ~CValueBase() {}
	};

	struct CValueDsid : public CValueBase
	{
		DSegmentId_t mDsid;
		~CValueDsid() final
		{}
	};

	CUserCommand() : m_CmdType(UserCmdType_E::UNKNOWN)
	{}

	~CUserCommand()
	{
		Reset();
	}

	UserCmdType_E Parse(const CString& rCmd)
	{
		m_CmdType = UserCmdType_E::UNKNOWN;
		if (IsIdenticalToken(_T("print json info"), rCmd))
		{
			m_CmdType = UserCmdType_E::PRINT_JSON_INFO;
		}
		else if (IsIdenticalToken(_T("print dsid # from info"), rCmd))
		{
			m_CmdType = UserCmdType_E::PRINT_DSID_FROM_INFO;
		}
		else if (IsIdenticalToken(_T("print dsid # all parent cost"), rCmd))
		{
			m_CmdType = UserCmdType_E::PRINT_DSID_ALL_PARENT_COST;
		}

		return m_CmdType;
	}

	DSegmentId_t GetParsedDsid(UserCmdType_E cmdtyp) const
	{
		DSegmentId_t val_dsid = 0;

		switch (cmdtyp)
		{
		case UserCmdType_E::UNKNOWN:
			break;
		case UserCmdType_E::PRINT_DSID_ALL_PARENT_COST:
			if (m_CmdType == cmdtyp)
			{
				CValueDsid* pDsid = dynamic_cast<CValueDsid*>(m_ValueList.GetHead());
				ASSERT_POINTER(pDsid, CValueDsid);
				val_dsid = pDsid->mDsid;
			}
			break;
		case UserCmdType_E::PRINT_DSID_FROM_INFO:
			if (m_CmdType == cmdtyp)
			{
				CValueDsid* pDsid = dynamic_cast<CValueDsid*>(m_ValueList.GetHead());
				ASSERT_POINTER(pDsid, CValueDsid);
				val_dsid = pDsid->mDsid;
			}
			break;
		case UserCmdType_E::MAX:
			break;
		default:
			break;
		}

		return val_dsid;
	}

	BOOL IsIdenticalToken(LPCTSTR pStrA, LPCTSTR pStrB, BOOL bIgnorCase = TRUE)
	{
		if (!pStrA || !pStrA)
		{
			return FALSE;
		}

		Reset();

		CString delimit(_T(", .\t\r\n")), strDest(pStrA), strSrc(pStrB);
		int dst_pos = 0, src_pos = 0;
		BOOL isIdentical = TRUE;

		CString srctkn = strSrc.Tokenize(delimit, src_pos);
		while (srctkn != _T(""))
		{
			m_SrcTokenList.AddTail(srctkn);
			srctkn = strSrc.Tokenize(delimit, src_pos);
		}

		CString dsttkn = strDest.Tokenize(delimit, dst_pos);
		while (dsttkn != _T(""))
		{
			m_FmtTokenList.AddTail(dsttkn);
			dsttkn = strDest.Tokenize(delimit, dst_pos);
		}

		if (m_SrcTokenList.GetCount() != m_FmtTokenList.GetCount())
		{
			return FALSE;
		}

		POSITION pos_src = m_SrcTokenList.GetHeadPosition();
		POSITION pos_dst = m_FmtTokenList.GetHeadPosition();
		while (pos_src)
		{
			srctkn = m_SrcTokenList.GetNext(pos_src);
			dsttkn = m_FmtTokenList.GetNext(pos_dst);

			if (bIgnorCase)
			{
				srctkn.MakeLower();
				dsttkn.MakeLower();
			}

			if (_T("#") == dsttkn && srctkn.SpanExcluding(_T("0123456789")).IsEmpty())
			{
				if (!m_PreSrcToken.IsEmpty() && _T("dsid") == m_PreSrcToken)
				{
					m_ValueList.AddHead(CreateValueDsid(_tstoi64(srctkn)));
				}
				continue;
			}

			if (srctkn != dsttkn)
			{
				return FALSE;
			}

			m_PreSrcToken = srctkn;
		}

		return TRUE;
	}

	CValueDsid* CreateValueDsid(DSegmentId_t val)
	{
		CValueDsid* pDsid = new CValueDsid;
		pDsid->mValType = VT_DSID;
		pDsid->mDsid = val;
		return pDsid;
	}

	void Reset()
	{
		while (!m_ValueList.IsEmpty())
		{
			CValueBase* pVal = m_ValueList.RemoveHead();
			delete pVal;
		}

		m_CmdType = UserCmdType_E::UNKNOWN;
		m_PreSrcToken.Empty();
		m_SrcTokenList.RemoveAll();
		m_FmtTokenList.RemoveAll();
	}

private:
	CTypedPtrList<CPtrList, CValueBase*> m_ValueList;
	CStringList m_SrcTokenList;
	CStringList m_FmtTokenList;
	CString m_PreSrcToken;
	UserCmdType_E m_CmdType;
};



namespace nsWorkThread
{
	class CSessionInfoBas : public nsRefobj::CReferenceBase
	{
	public:
		CSessionInfoBas() = delete;
		CSessionInfoBas(size_t cmd) : mCmd(cmd)
		{}

		virtual bool IsValid() = 0;
		const size_t mCmd;
	protected:
		virtual ~CSessionInfoBas() {}
	};

	struct CParameter
	{
		HWND m_hwndInfo;
		HANDLE m_hEventStart;
		HANDLE m_hEventDone;
		HANDLE m_hEventKill;
		HANDLE m_hEventDead;
		CSessionInfoBas* m_pSessInfo;
		nsRefobj::Handle_t m_hRequire;
		nsRefobj::Handle_t m_hAcquire;
	};

	void FinalDiscardRef(nsRefobj::Handle_t& h)
	{
		ASSERT(!nsRefobj::IsMultiRefered(h));
		nsRefobj::Discard(h);
		h = nsRefobj::InvalidReference;
	}

	void DiscardMultiRef(nsRefobj::Handle_t &h, bool bInvalidate = false)
	{
		ASSERT(nsRefobj::IsMultiRefered(h));
		nsRefobj::Discard(h);
		if (bInvalidate)
		{
			h = nsRefobj::InvalidReference;
		}
	}

	class CAcqPrepDisp : public CSessionInfoBas
	{
	public:
		static nsRefobj::Handle_t CreateInstance()
		{
			CAcqPrepDisp* ins = new CAcqPrepDisp();
			return ins->Refer();
		}
		bool IsValid() final
		{
			return WORK_CMD_E::ACQ_PREPARE_DISP == CvtToCmd(mCmd);
		}

		bool Initialize(HDC hDC)
		{
			m_hdcDest = hDC;
			nsDispScope::Handle_t hDspScp = InitDisplayScope();
			ASSERT(hDspScp);

			nsDocHidden::CRevTable* pRevTab = nsRefobj::Refer<nsDocHidden::CRevTable>(m_hrefRevTable);
			if (pRevTab)
			{
				VERIFY(pRevTab->BuildQuery());
				pRevTab->Release();
			}

			SIZE sizMesh = { 1000, 800 };
			return InitSearchBranchLayer(hDspScp, sizMesh)
				&& InitRouteLayer(hDspScp, sizMesh);
		}

		CSearchBranchLayer::RefHandle_t GetRutSrchBrchLayer() const
		{
			return m_pSrchBrchSurf->Handle();
		}

		CRouteLayer::RefHandle_t GetRouteLayer() const
		{
			return m_pRouteSurf->Handle();
		}

		nsRefobj::Handle_t GetRevTable() const
		{
			return m_hrefRevTable;
		}
	private:
		CAcqPrepDisp() : CSessionInfoBas(CvtToCmd(WORK_CMD_E::ACQ_PREPARE_DISP))
			, m_hdcDest(NULL)
			, m_pDispScope(NULL)
			, m_pSrchBrchSurf(NULL)
			, m_pRouteSurf(NULL)
		{
			m_hrefRevTable = nsDocHidden::CRevTable::CreateReference();
			ASSERT(m_hrefRevTable);
		}

		~CAcqPrepDisp()
		{
			if (m_pSrchBrchSurf)
			{
				m_pSrchBrchSurf->Release();
			}

			if (m_pRouteSurf)
			{
				m_pRouteSurf->Release();
			}

			if (m_pDispScope)
			{
				m_pDispScope->Release();
			}
			//nsRefobj::Discard(m_hrefRevTable);

			nsWorkThread::DiscardMultiRef(m_hrefRevTable);
		}

		nsDispScope::Handle_t InitDisplayScope()
		{
			nsDispScope::Handle_t hDispScope = CDisplayScope::Create();
			ASSERT(hDispScope);

			m_pDispScope = CDisplayScope::Refer(hDispScope);
			ASSERT(m_pDispScope);

			JsonRouteSrchGeoInfo GeoInfo;
			if (Json_getRouteSrchGeoInfo(&GeoInfo))
			{
				ASSERT_POINTER(m_pDispScope, CDisplayScope);
				m_pDispScope->SetGeoScope(GeoInfo.mMaxFromRectTopLeft, GeoInfo.mMaxFromRectBtmRight);
			}

			return hDispScope;
		}

		bool InitSearchBranchLayer(nsDispScope::Handle_t hDspScp, const SIZE &sizMesh)
		{
			auto hRef = CSearchBranchLayer::CreateInstance(hDspScp);
			ASSERT(hRef);

			m_pSrchBrchSurf = CSearchBranchLayer::Refer(hRef);
			ASSERT_POINTER(m_pSrchBrchSurf, CSearchBranchLayer);

			CSearchBranchLayer& layer = *m_pSrchBrchSurf;
			return !!layer.Initialize(sizMesh, m_hdcDest);
		}

		bool InitRouteLayer(nsDispScope::Handle_t hDspScp, const SIZE &sizMesh)
		{
			ASSERT_POINTER(hDspScp, CDisplayScope);
			auto hRef = CRouteLayer::CreateInstance(hDspScp);

			m_pRouteSurf = CRouteLayer::Refer(hRef);
			ASSERT_POINTER(m_pRouteSurf, CRouteLayer);

			CRouteLayer& layer = *m_pRouteSurf;
			return !!layer.Initialize(sizMesh, m_hdcDest);
		}

	private:
		CDisplayScope* m_pDispScope;
		CSearchBranchLayer* m_pSrchBrchSurf;
		CRouteLayer* m_pRouteSurf;
		nsRefobj::Handle_t m_hrefRevTable;
		HDC m_hdcDest;
	};
};

class CReqPrepDisp : public nsWorkThread::CSessionInfoBas
{
public:
	CReqPrepDisp() : CSessionInfoBas(CvtToCmd(WORK_CMD_E::REQ_PREPARE_DISP))
	{}

	bool IsValid()
	{
		return true;
	}
};

class CRoudlerDoc::CPrivate
{
public:
	CPrivate() 
		: m_hJsonLoader(MY_INVALID_HANDLE)
		, m_isDispScopeReady(FALSE)
		, m_isBusyWork(false)
		, m_pRevTab(NULL)
		, m_pThreadWorker(NULL)
	{
		m_hEventStartWork = ::CreateEvent(NULL, FALSE, FALSE, NULL);	// auto reset, initial FALSE
		m_hEventWorkDone = ::CreateEvent(NULL, TRUE, TRUE, NULL);	// manual reset, initial TRUE
		m_hEventKillWorker = ::CreateEvent(NULL, FALSE, FALSE, NULL);	// auto reset, initial FALSE
		m_hEventWorkerKilled = ::CreateEvent(NULL, TRUE, FALSE, NULL);	// manual reset, initial FALSE


		m_WorkerParam.m_hEventStart = m_hEventStartWork;
		m_WorkerParam.m_hEventDone = m_hEventWorkDone;
		m_WorkerParam.m_hEventKill = m_hEventKillWorker;
		m_WorkerParam.m_hEventDead = m_hEventWorkerKilled;
		m_WorkerParam.m_pSessInfo = NULL;
		m_WorkerParam.m_hwndInfo = NULL;
		m_WorkerParam.m_hAcquire = nsRefobj::InvalidReference;
		m_WorkerParam.m_hRequire = nsRefobj::InvalidReference;

		m_curPrintSrchBrchInfo.isReadyPrint = false;
		m_curPrintSrchBrchInfo.isProcessing = false;
	}

	~CPrivate()
	{
		DWORD dwExitCode = 0;
		if (NULL != m_pThreadWorker && ::GetExitCodeThread(m_pThreadWorker->m_hThread, &dwExitCode) && STILL_ACTIVE == dwExitCode)
		{
			::SetEvent(m_hEventKillWorker);
			::SetEvent(m_hEventStartWork);
			::WaitForSingleObject(m_hEventWorkerKilled, INFINITE);
		}
		
		RemoveLayerAll();

		if (m_pRevTab)
		{
			m_pRevTab->Release();
		}
	}

	BOOL CreateWorkerOnce()
	{
		if (NULL == m_pThreadWorker)
		{
			m_pThreadWorker = ::AfxBeginThread(WorkerProc, &m_WorkerParam);
		}

		ASSERT(m_pThreadWorker);

#if 0	// actually g_hViewWnd on new document
		if (NULL == m_WorkerParam.m_hwndInfo)
		{
			m_WorkerParam.m_hwndInfo = g_hViewWnd;
		}
#endif

		return (NULL != m_pThreadWorker);
	}

	BOOL InitInfoHanldleOnce()
	{
		if (!m_WorkerParam.m_hwndInfo)
		{
			m_WorkerParam.m_hwndInfo = g_hViewWnd;
		}

		return (NULL != m_WorkerParam.m_hwndInfo);
	}

	bool AsynStartWork()
	{
		InitInfoHanldleOnce();

		if (m_isBusyWork)
		{
			return false;
		}

		::ResetEvent(m_hEventKillWorker);
		::ResetEvent(m_hEventWorkDone);
		::SetEvent(m_hEventStartWork);

		return true;
	}

	bool RequirePrepareDisplay()
	{
		if (::WaitForSingleObject(m_hEventWorkDone, 0) == WAIT_OBJECT_0)
		{
			m_WorkerParam.m_pSessInfo = &m_ReqPrepDisp;
			return AsynStartWork();
		}

		return false;
	}

	static UINT WorkerProc(LPVOID pAnyParam)
	{
		ASSERT(pAnyParam);
		nsWorkThread::CParameter& param = *static_cast<nsWorkThread::CParameter*>(pAnyParam);
		CWnd* pInfoView = NULL;
		

		while (::WaitForSingleObject(param.m_hEventStart, INFINITE) == WAIT_OBJECT_0)
		{
			if (param.m_pSessInfo)
			{
				if (!pInfoView)
				{
					ASSERT(param.m_hwndInfo);
					pInfoView = CWnd::FromHandle(param.m_hwndInfo);
					pInfoView->PostMessage(WM_MYNOTIFY, E_WID_LIFE_CYCLE, (LPARAM)THD_WK_LC_E::PROC_START);
				}

				switch (CvtToCmd(param.m_pSessInfo->mCmd))
				{
				case WORK_CMD_E::REQ_PREPARE_DISP:
					if (true)
					{
						param.m_hAcquire = nsWorkThread::CAcqPrepDisp::CreateInstance();
						nsWorkThread::CAcqPrepDisp* pAcq = nsRefobj::Refer<nsWorkThread::CAcqPrepDisp>(param.m_hAcquire);

						ASSERT(g_hViewWnd);
						CWnd* pDest = CWnd::FromHandle(g_hViewWnd);
						if (pDest && pAcq->Initialize(pDest->GetDC()->m_hDC))
						{
							pInfoView->PostMessage(WM_MYNOTIFY, E_WID_LIFE_CYCLE, (LPARAM)THD_WK_LC_E::PROC_DISP_PREPARED);
						}
						else
						{
							pInfoView->PostMessage(WM_MYNOTIFY, E_WID_LIFE_CYCLE, (LPARAM)THD_WK_LC_E::PROC_DISP_PREPARE_FAILURE);
						}

						pAcq->Release();

						ASSERT(!nsRefobj::IsMultiRefered(param.m_hAcquire));
					}
					break;
				default:
					break;
				}
			}

			if (::WaitForSingleObject(param.m_hEventKill, 0) == WAIT_OBJECT_0)
			{
				break;
			}
		}

		::SetEvent(param.m_hEventDead);
		//pInfoView->PostMessage(WM_MYNOTIFY, E_WID_LIFE_CYCLE, (LPARAM)THD_WK_LC_E::PROC_END);
		return 0;
	}

	void RemoveLayer(CRoudlerSurface* pSurf)
	{
		if (pSurf)
		{
			if (pSurf->IsKindOf(RUNTIME_CLASS(CSearchBranchLayer)))
			{
				CSearchBranchLayer* pLayer = DYNAMIC_DOWNCAST(CSearchBranchLayer, pSurf);
				pLayer->Release();
			}
			else if (pSurf->IsKindOf(RUNTIME_CLASS(CRouteLayer)))
			{
				CRouteLayer* pLayer = DYNAMIC_DOWNCAST(CRouteLayer, pSurf);
				pLayer->Release();
			}
			else
			{
				ASSERT(FALSE);
			}
		}
	}

	void RemoveLayerAll()
	{
		while (!m_SurfList.IsEmpty())
		{
			CRoudlerSurface* pSurf = m_SurfList.RemoveHead();
			RemoveLayer(pSurf);
		}
	}

	void OnAcqPrepareDisplay()
	{
		TRACE0("OnAcqPrepareDisplay() enter.\n");
		if (NULL != m_WorkerParam.m_pSessInfo 
			&& WORK_CMD_E::ACQ_PREPARE_DISP == CvtToCmd(m_WorkerParam.m_pSessInfo->mCmd)
			//&& m_WorkerParam.m_pSessInfo->IsValid()
			&& nsRefobj::IsValid(m_WorkerParam.m_hAcquire))
		{
			nsWorkThread::CAcqPrepDisp* pAcq = nsRefobj::Refer<nsWorkThread::CAcqPrepDisp>(m_WorkerParam.m_hAcquire);;
			if (pAcq)
			{
				RemoveLayerAll();

				auto hBrchLayer = pAcq->GetRutSrchBrchLayer();
				m_SurfList.AddTail(CSearchBranchLayer::Refer(hBrchLayer));

				auto hRouteLayer = pAcq->GetRouteLayer();
				m_SurfList.AddTail(CRouteLayer::Refer(hRouteLayer));

				nsRefobj::Handle_t hRef = pAcq->GetRevTable();
				m_pRevTab = (nsRefobj::InvalidReference == hRef) ? NULL : nsRefobj::Refer<nsDocHidden::CRevTable>(hRef);
				if (m_pRevTab)
				{
					m_curPrintSrchBrchInfo.isReadyPrint = m_pRevTab->GetBranchInfoNum() > 0;
				}

				ASSERT(nsRefobj::IsMultiRefered(hRef));

				pAcq->Release();
			}

			m_WorkerParam.m_pSessInfo = NULL;

			nsWorkThread::FinalDiscardRef(m_WorkerParam.m_hAcquire);
		}

		TRACE0("OnAcqPrepareDisplay() leave.\n");
	}

	void OnAcqFailedPrepDispaly()
	{
		nsWorkThread::CAcqPrepDisp* pAcq = nsRefobj::IsValid(m_WorkerParam.m_hAcquire)
			? nsRefobj::Refer<nsWorkThread::CAcqPrepDisp>(m_WorkerParam.m_hAcquire)
			: NULL;
		if (pAcq)
		{
			nsRefobj::Handle_t hRef = pAcq->GetRevTable();
			m_pRevTab = (nsRefobj::InvalidReference == hRef) ? NULL : nsRefobj::Refer<nsDocHidden::CRevTable>(hRef);
			if (m_pRevTab)
			{
				m_curPrintSrchBrchInfo.isReadyPrint = m_pRevTab->GetBranchInfoNum() > 0;
			}

			pAcq->Release();
		}

		nsWorkThread::FinalDiscardRef(m_WorkerParam.m_hAcquire);
	}

	BOOL ParseUserCommand(const CString& rCmd)
	{
		BOOL cmd_ok = TRUE;

		CWnd* pView = CWnd::FromHandle(g_hViewWnd);
		UserCmdType_E cmdtyp = m_UserCmd.Parse(rCmd);
		switch (cmdtyp)
		{
		case UserCmdType_E::PRINT_JSON_INFO:
			pView->PostMessage(WM_MYNOTIFY, E_NID_DOC_PARSER_PRINT_JSONINFO);
			break;
		case UserCmdType_E::PRINT_DSID_FROM_INFO:
			m_curPrintSrchBrchInfo.mDisd = m_UserCmd.GetParsedDsid(cmdtyp);
			pView->PostMessage(WM_MYNOTIFY, E_NID_DOC_PARSER_PRINT_BRANCH_INFO);
			break;
		case UserCmdType_E::PRINT_DSID_ALL_PARENT_COST:
			m_curPrintSrchBrchInfo.mDisd = m_UserCmd.GetParsedDsid(cmdtyp);
			pView->PostMessage(WM_MYNOTIFY, E_NID_DOC_PARSER_PRINT_PARENT_COST_INFO);
			break;
		default:
			cmd_ok = false;
			break;
		}

		return cmd_ok;
	}

	bool IsReadyJsonPrintInfo()
	{
		return Json_getRouteSrchBranchNum() > 0;
	}

	void OnCmdPrintBranchInfo()
	{
		CString info;
		nsDocHidden::CRevTable::CFromQInfo qinf;
		if (m_pRevTab->QueryBranchInfo(m_curPrintSrchBrchInfo.mDisd, qinf))
		{
			JsonFromInfo fi;
			VERIFY(Json_getSrchBrchFromInfo(qinf.mIdx, &fi));

			CString strConflictInfo;
			if (qinf.mRepcnt > 0)
			{
				strConflictInfo.Format(_T("conflict count %u! "), qinf.mRepcnt);
			}
			else
			{
				strConflictInfo = _T("No conflict. ");
			}

			info.Format(INFO_SEPERATOR_LINE
				_T("\tFrom DSegID %llu, NAVINFO Link ID %u\r\n")
				_T("\tis %s search, is %s,\r\n")
				_T("\t%s\r\n")
				_T("\tout link number %d."),
				fi.m_dsid,
				fi.m_nilid,
				fi.m_isRevSrh ? _T("Backword") : _T("Foreward"),
				strConflictInfo.LockBuffer(),
				fi.m_isSuper ? _T("SUPER") : _T("not super"), 
				fi.m_outnum);
			strConflictInfo.ReleaseBuffer();

			SendInfo(info.LockBuffer());
			info.ReleaseBuffer();
		}
		else
		{
			info.Format(_T("DSID %llu not find on query!"), m_curPrintSrchBrchInfo.mDisd);
			SendInfo(info.LockBuffer());
			info.ReleaseBuffer();
		}
	}

	void OnCmdPrintParentCostInfo()
	{
		CString info;
		JsonFromInfo fi;
		JsonOutInfo OutInfo;
		nsDocHidden::CRevTable::CParentBranchIndex ParentIdx;
		nsDocHidden::CRevTable::QParentBranchEle_t* pParentLst = NULL;
		CList<nsDocHidden::CRevTable::CParentBranchIndex> IdxList;
		CList<JsonOutInfo> OutInfLst;
		bool isBackward = false;

		struct CCombinedInfo
		{
			JsonFromInfo mFromInfo;
			JsonOutInfo mOutInfo;
			RouteCost_t mArrivalCost;
		}CombInfo;

		CList<CCombinedInfo> CombInfoList;
		RouteCost_t ArrivalCost = 0;

		DSegmentId_t CurDSegId = m_curPrintSrchBrchInfo.mDisd;

		while (m_pRevTab->QueryParentBranchInfo(CurDSegId, pParentLst))
		{
			ASSERT(pParentLst);
			nsDocHidden::CRevTable::QParentBranchEle_t& rList = *pParentLst;
			if (1 == rList.GetCount())
			{
				ParentIdx = rList.GetHead();
				if (Json_getSrchBrchFromInfo(ParentIdx.mBranchIdx, &fi))
				{
					if (IdxList.IsEmpty())
					{
						isBackward = fi.m_isRevSrh;
					}

					if (Json_getBranchOutInfo(ParentIdx.mBranchIdx, ParentIdx.mOutIdx, &OutInfo))
					{
						OutInfLst.AddHead(OutInfo);
						IdxList.AddHead(ParentIdx);

						CombInfo.mFromInfo = fi;
						CombInfo.mOutInfo = OutInfo;
						CombInfoList.AddHead(CombInfo);
					}

					CurDSegId = fi.m_dsid;
				}
			}
			else if (rList.GetCount() > 1)
			{
				info.Format(INFO_SEPERATOR_LINE
					_T("\tThere are %u parent candicates links"), rList.GetCount());
				SendInfo(info.LockBuffer());
				info.ReleaseBuffer();

				size_t CandiCnt = 0;
				POSITION pos = rList.GetHeadPosition();
				while (pos != NULL)
				{
					ParentIdx = rList.GetNext(pos);
					if (Json_getSrchBrchFromInfo(ParentIdx.mBranchIdx, &fi) 
						&& Json_getBranchOutInfo(ParentIdx.mBranchIdx, ParentIdx.mOutIdx, &OutInfo))
					{
						info.Format(_T("\tcandi[%u]: DSegID %llu, NavInfoLinkID %u, cost %u, %s\r\n")
							_T("\t%s from %s DSegID %llu, NIL ID %u"), 
							CandiCnt++,
							OutInfo.m_dsid,
							OutInfo.m_nilid,
							OutInfo.m_cost,
							OutInfo.m_isSuper ? _T("super link") : _T("Not super"),
							fi.m_isRevSrh ? _T("backward, ") : _T("foreward,"),
							fi.m_isSuper ? _T("super") : _T("lower"),
							fi.m_dsid,
							fi.m_nilid);

						SendInfo(info.LockBuffer());
						info.ReleaseBuffer();
					}
				}

				break;
			}
			else
			{
				ASSERT(FALSE);
				break;
			}
		}

		info.Format(INFO_SEPERATOR_LINE
			_T("\tparent link number is %u."),
			CombInfoList.GetCount());
		SendInfo(info.LockBuffer());
		info.ReleaseBuffer();

		size_t PrintCnt = 0;

		POSITION CombPos = CombInfoList.GetHeadPosition();
		while (CombPos)
		{
			CombInfo = CombInfoList.GetNext(CombPos);
			ArrivalCost += CombInfo.mOutInfo.m_cost;

			info.Format(_T("\toutlink_info[%u], DSegID %llu, NIL ID %u\r\n")
				_T("\tlink cost %u, arrival cost %u"),
				
				PrintCnt++,
				CombInfo.mOutInfo.m_dsid,
				CombInfo.mOutInfo.m_nilid,
				CombInfo.mOutInfo.m_cost,
				ArrivalCost
				);

			SendInfo(info.LockBuffer());
			info.ReleaseBuffer();
		}
	}

	bool IsReady4PrintBranchInfo() const
	{
		return m_curPrintSrchBrchInfo.isReadyPrint;
	}

	void OnCmdPrintJsonInfo()
	{
		CString info;
		size_t branchnum = Json_getRouteSrchBranchNum();
		if (branchnum > 0)
		{
			info.Format(_T("Search branch number is %lu."), branchnum);
			SendInfo(info.LockBuffer());
			info.ReleaseBuffer();
		}

		size_t routenum = Json_getRouteNum();
		if (routenum > 0)
		{
			info.Format(_T("Route number is %lu."), routenum);
			SendInfo(info.LockBuffer());
			info.ReleaseBuffer();
		}

		if (info.IsEmpty())
		{
			SendInfo(_T("Nothing to tell! 'cause NO info available"));
		}
	}

	void SendInfo(LPTSTR nick, LPTSTR msg) 
	{
		CMainFrame* pFrm = DYNAMIC_DOWNCAST(CMainFrame, ::AfxGetMainWnd());
		ASSERT_VALID(pFrm);

		CInfoView* pInfo = DYNAMIC_DOWNCAST(CInfoView, pFrm->m_wndSpliteMsg.GetPane(0, 1));
		ASSERT_VALID(pFrm);

		CString line;
		if (msg && nick)
		{
			line.Format(IDS_FMT2_INFOLINE, nick, msg);
			pInfo->Message(line);

			if (CString(nick) == "user")
			{
				TRACE1("DOC : user command : %s\n", msg);
				if (!ParseUserCommand(CString(msg)))
					SendInfo(_T("I don't understand!"));
			}
		}
		else if (msg)
		{
			line.Format(IDS_FMT1_INFOLINE, msg);
			pInfo->Message(line);
		}
	}

	void SendInfo(LPTSTR msg)
	{
		return SendInfo(g_szDocNick, msg);
	}
public: // attributes
	CArray<POINT> m_aryFromGeoPointPair;

	struct GEO_RECT
	{
		LONG Width, Height;
		POINT TopLeft, BottomRight;
	}m_geoFromRect;

	BOOL m_isDispScopeReady;
	SurfaceList_t m_SurfList;
private:
	bool m_isBusyWork;
	struct tagPrintSearchBranchInfo
	{
		bool isReadyPrint;
		bool isProcessing;
		bool isMultiInfo;
		DSegmentId_t mDisd;
	}m_curPrintSrchBrchInfo;

	MyHandle_t m_hJsonLoader;
	CWinThread* m_pThreadWorker;
	HANDLE m_hEventStartWork;
	HANDLE m_hEventKillWorker;
	HANDLE m_hEventWorkDone;
	HANDLE m_hEventWorkerKilled;
	CReqPrepDisp m_ReqPrepDisp;
	CUserCommand m_UserCmd;
	nsDocHidden::CRevTable* m_pRevTab;
	nsWorkThread::CParameter m_WorkerParam; // shared by main and worker thread
};

// CRoudlerDoc construction/destruction
CRoudlerDoc::CRoudlerDoc() : mp(new CPrivate)
{
	// TODO: add one-time construction code here
}

CRoudlerDoc::~CRoudlerDoc()
{
	delete mp;
}

BOOL CRoudlerDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)
	mp->CreateWorkerOnce();

	return TRUE;
}


// CRoudlerDoc serialization
void CRoudlerDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
		CString strPath(ar.GetFile()->GetFilePath());
		AsynImportJson(strPath.GetBuffer());
		strPath.ReleaseBuffer();
	}
}

#ifdef SHARED_HANDLERS

// Support for thumbnails
void CRoudlerDoc::OnDrawThumbnail(CDC& dc, LPRECT lprcBounds)
{
	// Modify this code to draw the document's data
	dc.FillSolidRect(lprcBounds, RGB(255, 255, 255));

	CString strText = _T("TODO: implement thumbnail drawing here");
	LOGFONT lf;

	CFont* pDefaultGUIFont = CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));
	pDefaultGUIFont->GetLogFont(&lf);
	lf.lfHeight = 36;

	CFont fontDraw;
	fontDraw.CreateFontIndirect(&lf);

	CFont* pOldFont = dc.SelectObject(&fontDraw);
	dc.DrawText(strText, lprcBounds, DT_CENTER | DT_WORDBREAK);
	dc.SelectObject(pOldFont);
}

// Support for Search Handlers
void CRoudlerDoc::InitializeSearchContent()
{
	CString strSearchContent;
	// Set search contents from document's data. 
	// The content parts should be separated by ";"

	// For example:  strSearchContent = _T("point;rectangle;circle;ole object;");
	SetSearchContent(strSearchContent);
}

void CRoudlerDoc::SetSearchContent(const CString& value)
{
	if (value.IsEmpty())
	{
		RemoveChunk(PKEY_Search_Contents.fmtid, PKEY_Search_Contents.pid);
	}
	else
	{
		CMFCFilterChunkValueImpl *pChunk = NULL;
		ATLTRY(pChunk = new CMFCFilterChunkValueImpl);
		if (pChunk != NULL)
		{
			pChunk->SetTextValue(PKEY_Search_Contents, value, CHUNK_TEXT);
			SetChunkValue(pChunk);
		}
	}
}

#endif // SHARED_HANDLERS

// CRoudlerDoc diagnostics

#ifdef _DEBUG
void CRoudlerDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CRoudlerDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CRoudlerDoc commands


BOOL CRoudlerDoc::OnSaveDocument(LPCTSTR lpszPathName)
{
	// TODO: Add your specialized code here and/or call the base class

	//MessageBox(NULL, _T("Save not supported!"), _T("Warning"), MB_OK | MB_ICONWARNING);

	return FALSE;
	//return CDocument::OnSaveDocument(lpszPathName);
}

CPoint CRoudlerDoc::convertFromGeoPnt2viewPnt(CPoint const &ptSrc, CRect const &rec)
{
	CPoint pt = ptSrc - mp->m_geoFromRect.TopLeft;
	pt.y = mp->m_geoFromRect.Height - pt.y;

	return pt;
}

// 
IMPLEMENT_SERIAL(CFromDSegment, CObject, 1)

CFromDSegment::CFromDSegment()
{
}

void CFromDSegment::Serialize(CArchive&)
{
}

void CRoudlerDoc::DeleteContents()
{
	// TODO: Add your specialized code here and/or call the base class
	CDocument::DeleteContents();
}


void CRoudlerDoc::OnFileImportJson()
{
	CFileDialog dlg(TRUE);
	if (IDOK == dlg.DoModal())
	{
		CString strPath(dlg.GetPathName());

		AsynImportJson(strPath.GetBuffer());
		strPath.ReleaseBuffer();
	}
	else
	{
		SendInfo(g_szDocNick, _T("cancel select json"));
	}
}

// NOTE : notifyRouteSrchDataComplete will be called in std::thread_t;
// You have to notify target thread via the HWND
void notifyRouteSrchDataComplete(MyHandle_t)
{
	BOOL OK = ::PostMessage(g_hViewWnd, WM_MYNOTIFY, E_NID_JSON_COMPLETE, 0);
	if (!OK) TRACE0("POST complete message failed!\n");
}

int CRoudlerDoc::asynLoadRouteSearchData(LPCTSTR lpszPathName)
{
	ASSERT(lpszPathName);
	USES_CONVERSION;
	mp->m_isDispScopeReady = FALSE;

	Json_setRouteSearchDataCallBack(notifyRouteSrchDataComplete);
	(void)Json_asynLoadRouteSearchData(T2A(lpszPathName));

	return 0;
}

void CRoudlerDoc::TestOnDraw(CDC* pDC, LPRECT pRect)
{
	//mp->m_prepare.Draw(pDC, pRect);
	TRACE2("draw orgin x:%d, y%d\n", pRect->left, pRect->top);
	auto& surfList = mp->m_SurfList;
	for (POSITION pos = surfList.GetHeadPosition(); pos != NULL;)
	{
		CRoudlerSurface* pSurf = surfList.GetNext(pos);
		ASSERT(pSurf);
		pSurf->Draw(pDC, pRect);
	}
}

LRESULT CRoudlerDoc::ProcMyNotification(WPARAM wParam, LPARAM lParam)
{
	CString info;
	auto mid = wParam;
	switch (mid)
	{
	case E_NID_JSON_COMPLETE:
	{
		SendInfo(_T("JSON file loaded and asynchronic prepare dispaly."));
		mp->RequirePrepareDisplay();
		break;
	}
	case E_NID_TEST:
	{
		SendInfo(_T("MY TEST."));
		break;
	}
	case E_TID_DISP_PREP_RUTLNK_SHPPNT_NUM:
	{
		size_t shpPntNum = lParam;
		info.Format(_T("Display prepare : shape point number is %lu."), shpPntNum);
		SendInfo(info.LockBuffer());
		info.ReleaseBuffer();
	}
	break;
	case E_NID_DOC_PARSER_PRINT_PARENT_COST_INFO:
		if (mp->IsReady4PrintBranchInfo())
		{
			mp->OnCmdPrintParentCostInfo();
		}
		else
		{
			SendInfo(_T("Route Search branch info is not ready!"));
		}
		break;
	case E_NID_DOC_PARSER_PRINT_BRANCH_INFO:
		if (mp->IsReady4PrintBranchInfo())
		{
			mp->OnCmdPrintBranchInfo();
		}
		else
		{
			SendInfo(_T("Route Search branch info is not ready!"));
		}
		break;
	case E_NID_DOC_PARSER_PRINT_JSONINFO:
		if (mp->IsReadyJsonPrintInfo())
		{
			mp->OnCmdPrintJsonInfo();
		}
		else
		{
			SendInfo(_T("Json information is not ready!"));
		}
		break;
	case E_WID_LIFE_CYCLE:
		if (lParam > 0)
		{
			THD_WK_LC_E eState = static_cast<THD_WK_LC_E>(lParam);
			switch (eState)
			{
			case THD_WK_LC_E::PROC_START:
				SendInfo(_T("[WT][LC]worker thread proc start."));
				break;
			case THD_WK_LC_E::PROC_END:
				SendInfo(_T("[WT][LC]worker thread proc end."));
				break;
			case THD_WK_LC_E::PROC_DISP_PREPARED:
				SendInfo(_T("[WT][LC]display prepare start."));
				mp->m_isDispScopeReady = TRUE;
				mp->OnAcqPrepareDisplay();
				SendInfo(_T("[WT][LC]display prepare is over."));
				for (POSITION pos = GetFirstViewPosition(); pos != NULL;)
				{
					CView* pVw = GetNextView(pos);
					pVw->Invalidate();
					break;
				}
				break;
			case THD_WK_LC_E::PROC_DISP_PREPARE_FAILURE:
				if (mp->IsReadyJsonPrintInfo())
				{
					SendInfo(_T("[WT][LC]display prepare failed! But you can check the following JSON info"));
					mp->OnCmdPrintJsonInfo();
					mp->OnAcqFailedPrepDispaly();
				}
				else
				{
					SendInfo(_T("[WT][LC]display prepare failed and the JSON info is not ready yet!"));
				}
				break;
			default:
				break;
			}
		}
		break;
	default:
		break;
	}

	return 0;
}


BOOL CRoudlerDoc::CanCloseFrame(CFrameWnd* pFrame)
{
	// TODO: Add your specialized code here and/or call the base class
	Json_finalize();
	return CDocument::CanCloseFrame(pFrame);
}


void CRoudlerDoc::OnFinalRelease()
{
	// TODO: Add your specialized code here and/or call the base class

	CDocument::OnFinalRelease();
}


// send message
void CRoudlerDoc::SendInfo(LPTSTR nick, LPTSTR msg)
{
	return mp->SendInfo(nick, msg);
}

void CRoudlerDoc::SendInfo(LPTSTR msg)
{
	return mp->SendInfo(msg);
}

// check ready to display
BOOL CRoudlerDoc::IsDispReady() const
{
	return mp->m_isDispScopeReady;
}

// asynichronical import json
BOOL CRoudlerDoc::AsynImportJson(LPCTSTR path)
{
	CString strInfo;
	strInfo.Format(_T("Open \"%s\""), path);
	SendInfo(g_szDocNick, strInfo.LockBuffer());
	strInfo.ReleaseBuffer();

	return	asynLoadRouteSearchData(path);
}
