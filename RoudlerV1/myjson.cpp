// std header
#include <algorithm>
#include <array>
#include <fstream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <sstream>
#include <tuple>
#include <type_traits>
#include <vector>
// the 3rd header
#include "rapidjson/document.h"
#include "rapidjson/error/en.h"
#include "rapidjson/istreamwrapper.h"
// my header
#include "myhandle.h"
#include "myhobj.hpp"
#include "myjson.h"
#include "mythread.h"

using namespace CPP11_myhandle;
namespace CPP11_myjson
{
	enum {
		E_MID_NONE,
		E_MID_STARTLOAD_JSON,
		E_MID_EXIT,
		E_MID_MAX
	};

	class CJsonValue
	{
	public:
		CJsonValue() = delete;
		CJsonValue(const CJsonValue&) = delete;
		CJsonValue& operator=(const CJsonValue &) = delete;
		CJsonValue(rapidjson::Value& val) :m_value(val) {}
		~CJsonValue() {}
		bool isBool(const char* k) { return m_value.HasMember(k) && m_value[k].IsBool(); }
		bool isUint64(const char* k) { return m_value.HasMember(k) && m_value[k].IsUint64(); }
		bool isUint(const char* k) { return m_value.HasMember(k) && m_value[k].IsUint(); }
		bool isInt(const char* k) { return m_value.HasMember(k) && m_value[k].IsInt(); }
		bool isArray(const char* k) { return m_value.HasMember(k) && m_value[k].IsArray(); }
		bool getBool(const char* k, bool& v)
		{
			bool ok = m_value.HasMember(k) && m_value[k].IsBool();
			if (ok) v = m_value[k].GetBool();
			return ok;
		}

		bool getUint64(const char* k, uint64_t& v)
		{
			bool ok = m_value.HasMember(k) && m_value[k].IsUint64();
			if (ok) v = m_value[k].GetUint64();
			return ok;
		}

		bool getUint(const char* k, uint32_t& v)
		{
			bool ok = m_value.HasMember(k) && m_value[k].IsUint();
			if (ok) v = m_value[k].GetUint();
			return ok;
		}

		bool getInt(const char* k, int32_t& v)
		{
			bool ok = m_value.HasMember(k) && m_value[k].IsInt();
			if (ok) v = m_value[k].GetInt();
			return ok;
		}

		bool getArray(const char* k, rapidjson::Value* &v)
		{
			bool ok = m_value.HasMember(k) && m_value[k].IsArray();
			if (ok) v = &(m_value[k]);
			return ok;
		}
	private:
		rapidjson::Value& m_value;
	};


	class CRouteSearchData
	{
		struct CGarbo
		{
			CRouteSearchData* mData;
			CGarbo(CRouteSearchData* data) : mData(data) {}
			~CGarbo() { delete mData; }
		};
	public: // life cycle
		struct CActivate
		{
			CActivate() = delete;
			CActivate(MyThreadProc_t proc)
			{
				m_hThread = MyThread_create(proc, CRouteSearchData::getInstance());
			}
			~CActivate()
			{
				MyThread_join(m_hThread);
				MyThread_destroy(m_hThread);
			}
			MyHandle_t m_hThread;
		};

		static CRouteSearchData* getInstance()
		{
			static CRouteSearchData* ins = new CRouteSearchData;
			static CGarbo garbo(ins);
			return ins;
		}

		// operation : post msg
		bool postStartLoading(const char* json_path)
		{
			bool ret = false;
			if (json_path)
			{
				m_strJsonPath = json_path;
				MyMessage_t msg = { E_MID_STARTLOAD_JSON, nullptr };
				MyThread_sendMessage(m_activate.m_hThread, &msg);

				ret = true;
			}
			return ret;
		}

		void postExitLoop()
		{
			MyMessage_t msg = { E_MID_EXIT, nullptr };
			MyThread_sendMessage(m_activate.m_hThread, &msg);
		}

		void addFromInfo(bool isReverseSearch, bool isSuper, DSegmentId_t from_dsid, NavInfoLinkId_t from_nilid, JsonGeoPoint firstShpPt, JsonGeoPoint lastShpPt, size_t outnum)
		{
			int reserved = 0;
			m_fromInfo = std::make_tuple(isReverseSearch, isSuper, from_dsid, from_nilid, firstShpPt, lastShpPt, outnum);
		}

		void addOutInfo(bool isSuper, DSegmentId_t out_dsid, NavInfoLinkId_t out_nilid, RouteCost_t cost)
		{
			m_vecOutInfo.push_back(std::make_tuple(isSuper, out_dsid, out_nilid, cost));
		}

		bool startAddSearchBranch()
		{
			return (m_vecOutInfo.size() == m_iOutInfoStart);
		}

		bool endAddSearchBranch()
		{
			bool data_ok = false;

			bool from_isReverseSearch = false;
			bool from_isSuper = false;
			DSegmentId_t from_dsid = 0;
			NavInfoLinkId_t from_nilid = 0;
			size_t outnum = 0;
			std::pair<
				JsonGeoPoint, // first 
				JsonGeoPoint  // last
			> from_shpPt;

			std::tie(
				from_isReverseSearch,
				from_isSuper,
				from_dsid,
				from_nilid,
				from_shpPt.first,
				from_shpPt.second,
				outnum
				) = m_fromInfo;

			// check data integraty
			if ((m_vecOutInfo.size() - m_iOutInfoStart == outnum) && (0 != outnum))
			{
				m_vecBranch.push_back(std::make_tuple(
					from_isReverseSearch,
					from_isSuper,
					from_dsid,
					from_nilid,
					from_shpPt.first,
					from_shpPt.second,
					outnum,
					m_iOutInfoStart));

				m_iOutInfoStart += outnum;
				data_ok = true;
			}

			if (!data_ok)
			{
				m_vecOutInfo.erase(m_vecOutInfo.begin() + m_iOutInfoStart, m_vecOutInfo.end());
			}

			return data_ok;
		}

		bool procStartLoading()
		{
			auto report_err = [](const char*)
			{
			};

			auto loadOutInfo = [&](rapidjson::Value& rOutVal)
			{
				CJsonValue jvOut(rOutVal);
				bool is_outSuper;
				DSegmentId_t outDSegId;
				NavInfoLinkId_t outNid;
				RouteCost_t outCost;

				bool load_ok = false;
				if (jvOut.getBool("is_super", is_outSuper)
					&& jvOut.getUint64("out_dsid", outDSegId)
					&& jvOut.getUint("out_nilid", outNid)
					&& jvOut.getUint("out_cost", outCost))
				{
					// enter out DSegmentId
					addOutInfo(
						is_outSuper,
						outDSegId,
						outNid,
						outCost);

					load_ok = true;
				}

				return load_ok;
			};


			auto const xy_min = std::numeric_limits<uint32_t>::min();
			auto const xy_max = std::numeric_limits<uint32_t>::max();
			m_rectFrom = m_rectOut = { {xy_max, xy_max}, {xy_min, xy_min} };

			auto loadSearchBranch = [&](rapidjson::Value& rBchVal)
			{
				CJsonValue jvBrh(rBchVal);
				bool isRevSrh, is_fromSuper;
				DSegmentId_t fromDSegId;
				NavInfoLinkId_t fromNILid;
				rapidjson::Value* pOutAry;
				JsonGeoPoint firstShpPt, lastShpPt;
				int outnum;

				bool load_ok = false;
				if (jvBrh.getBool("is_revsrch", isRevSrh)
					&& jvBrh.getBool("from_isSuper", is_fromSuper)
					&& jvBrh.getUint64("from_dsid", fromDSegId)
					&& jvBrh.getUint("from_nilid", fromNILid)
					&& jvBrh.getInt("outnum", outnum)
					&& jvBrh.getUint("from_firstShpPtX", firstShpPt.x)
					&& jvBrh.getUint("from_firstShpPtY", firstShpPt.y)
					&& jvBrh.getUint("from_lastShpPtX", lastShpPt.x)
					&& jvBrh.getUint("from_lastShpPtY", lastShpPt.y)
					&& jvBrh.getArray("outary", pOutAry))
				{
					auto min_x = std::min(firstShpPt.x, lastShpPt.x);
					if (m_rectFrom.mTopLeft.x > min_x) m_rectFrom.mTopLeft.x = min_x;

					auto max_x = std::max(firstShpPt.x, lastShpPt.x);
					if (m_rectFrom.mBtmRight.x < max_x) m_rectFrom.mBtmRight.x = max_x;

					auto min_y = std::min(firstShpPt.y, lastShpPt.y);
					if (m_rectFrom.mTopLeft.y > min_y) m_rectFrom.mTopLeft.y = min_y;

					auto max_y = std::max(firstShpPt.y, lastShpPt.y);
					if (m_rectFrom.mBtmRight.y < max_y) m_rectFrom.mBtmRight.y = max_y;

					class CAdder{
					public:
						typedef CRouteSearchData* mp_t;
						CAdder(mp_t mp_) :mp(mp_) {}
						bool init() { return mp->startAddSearchBranch(); }
						~CAdder() { mp->endAddSearchBranch(); }
					private:
						mp_t mp;
					}adder(this); // avoid losing end action when exception

					if (adder.init())
					{
						addFromInfo(
							isRevSrh,
							is_fromSuper,
							fromDSegId,
							fromNILid,
							firstShpPt,
							lastShpPt,
							outnum);

						rapidjson::Value& ary = *pOutAry;
						rapidjson::SizeType outinfoIdx = 0;

						for (; outinfoIdx < ary.Size() &&
							loadOutInfo(ary[outinfoIdx]); outinfoIdx++)
						{}

						load_ok = (ary.Size() == outinfoIdx);
					}
				}

				return load_ok;
			};

			auto loadRutLnkShpPts = [&](rapidjson::Value& rShpPtAry, size_t& rBasIdx, size_t& rPntNum)->bool
			{
				JsonGeoPoint geoPt;
				rapidjson::SizeType ptidx = 0;
				size_t basidx = m_vecRutLnkShpPnt.size();
				for (; ptidx < rShpPtAry.Size(); ptidx++)
				{
					CJsonValue jvPt(rShpPtAry[ptidx]);
					if (jvPt.getUint("shppt_x", geoPt.x) && jvPt.getUint("shppt_y", geoPt.y))
					{
						m_vecRutLnkShpPnt.push_back(geoPt);
					}
				}

				bool load_ok = (ptidx > 0 && rShpPtAry.Size() == ptidx);
				if (load_ok)
				{
					rBasIdx = basidx;
					rPntNum = rShpPtAry.Size();
				}
				else if (ptidx > 0)
				{
					m_vecRutLnkShpPnt.erase(m_vecRutLnkShpPnt.begin() + basidx, m_vecRutLnkShpPnt.end());
				}

				return load_ok;
			};

			auto loadRutLink = [&](rapidjson::Value& rLnkVal)->bool
			{
				CJsonValue jvLnk(rLnkVal);
				rapidjson::Value* pShpPntAry = nullptr;
				DSegmentId_t dsid;
				size_t shppt_num, basidx;
				bool load_ok = false;
				if (jvLnk.getUint64("dsid", dsid)
					&& jvLnk.getArray("shppt_ary", pShpPntAry))
				{
					load_ok = loadRutLnkShpPts(*pShpPntAry, basidx, shppt_num);
					m_vecRouteLink.push_back(std::make_tuple(dsid, shppt_num, basidx));
				}

				return load_ok;
			};

			auto loadRoute = [&](rapidjson::Value& rRutVal)->bool
			{
				CJsonValue jvRut(rRutVal);
				RouteCost_t RutCost = 0;
				rapidjson::Value* pRutLnkAry;
				int lampNum = 0;

				bool load_ok = false;
				if (jvRut.getUint("cost", RutCost)
					&& jvRut.getInt("lightnum", lampNum)
					&& jvRut.getArray("linkary", pRutLnkAry))
				{

					rapidjson::Value& ary = *pRutLnkAry;
					rapidjson::SizeType linkIdx = 0;

					for (; linkIdx < ary.Size() && loadRutLink(ary[linkIdx]); linkIdx++)
					{}

					load_ok = (ary.Size() == linkIdx);
					if (load_ok)
					{
						size_t linknum = ary.Size();
						m_vecRouteInfo.push_back(std::make_tuple(
							linknum,
							RutCost,
							lampNum,
							m_iRutDsidStart));

						m_iRutDsidStart += linknum;
					}
				}

				return load_ok;
			};


			bool branch_ok = false;
			bool route_ok = false;
			do{
				if (m_strJsonPath.empty()){
					report_err("null path!");
					break;
				}

				std::ifstream ifs(m_strJsonPath.c_str());
				if (!ifs.is_open()) {
					report_err("open error!");
					break;
				}

				rapidjson::IStreamWrapper isw(ifs);
				rapidjson::Document d;

				d.ParseStream(isw);
				if (d.HasParseError())
				{
					report_err("parse error!");
					break;
				}

				const char* keyBranchAry = "ary_branch";
				if (d.HasMember(keyBranchAry) && d[keyBranchAry].IsArray())
				{
					rapidjson::Value& ary = d[keyBranchAry];
					rapidjson::SizeType branchIdx = 0;

					for (m_iOutInfoStart = 0; branchIdx < ary.Size() &&
						loadSearchBranch(ary[branchIdx]); branchIdx++)
					{}

					branch_ok = (ary.Size() == branchIdx);
				}

				const char* keyRouteAry = "ary_route";
				if (d.HasMember(keyRouteAry) && d[keyRouteAry].IsArray())
				{
					rapidjson::Value& ary = d[keyRouteAry];
					rapidjson::SizeType routeIdx = 0;
					for (m_iRutDsidStart = 0; routeIdx < ary.Size() && loadRoute(ary[routeIdx]); routeIdx++)
					{}

					route_ok = (ary.Size() == routeIdx);
				}
			} while (false);

			if (m_pfnNotifyCompleteLoading) 
				(*m_pfnNotifyCompleteLoading)(this);

			return branch_ok || route_ok;
		}

		void getRouteSrchGeoInfo(JsonRouteSrchGeoInfo& rInf)
		{
			rInf.mMaxFromRectTopLeft = m_rectFrom.mTopLeft;
			rInf.mMaxFromRectBtmRight = m_rectFrom.mBtmRight;
		}

		size_t getRouteNumber()
		{
			return m_vecRouteInfo.size();
		}

		void getRouteInfo(size_t rutidx, JsonRouteInfo& ri)
		{
			std::tie(
				ri.mLinkNum,
				ri.mCost,
				ri.mLampNum,
				m_curRutInfoCache.m_startLinkIdx
			) = m_vecRouteInfo.at(rutidx);
			m_curRutInfoCache.mRutIdx = rutidx;
			m_curRutInfoCache.mLinkNum = ri.mLinkNum;
		}

		void getRouteLink(size_t rutidx, size_t lnkidx, JsonRouteLink& rli)
		{
			if (rutidx == m_curRutInfoCache.mRutIdx && lnkidx < m_curRutInfoCache.mLinkNum)
			{
				// accessor helper
				m_curRutLnkCache.mLnkIdx = lnkidx;
				m_curRutLnkCache.mRutIdx = m_curRutInfoCache.mRutIdx;
				std::tie(m_curRutLnkCache.mDsid, 
					m_curRutLnkCache.mShpPntNum, 
					m_curRutLnkCache.mShpPntBasIdx
					) = m_vecRouteLink.at(m_curRutInfoCache.m_startLinkIdx + lnkidx);

				// output
				rli.mDsid = m_curRutLnkCache.mDsid;
				rli.mShpPntNum = m_curRutLnkCache.mShpPntNum;
			}
			else
			{
				std::ostringstream os;
				os << "route index " << rutidx << " is not the same as what the last getRouteNum got, ";
				os << "or the link index " << lnkidx << " is over range!";
				throw std::invalid_argument(os.str());
			}
		}

		void getRouteLinkShapePoint(size_t rutidx, size_t lnkidx, size_t pntidx, JsonGeoPoint& rPnt)
		{
			if (m_curRutLnkCache.mRutIdx == rutidx && m_curRutLnkCache.mLnkIdx == lnkidx && pntidx < m_curRutLnkCache.mShpPntNum)
			{
				rPnt = m_vecRutLnkShpPnt.at(m_curRutLnkCache.mShpPntBasIdx + pntidx);
			}
			else
			{
				std::ostringstream os;
				os << "expect route idx " << m_curRutLnkCache.mRutIdx << " link idx " << m_curRutLnkCache.mLnkIdx;
				os << " shape point number " << m_curRutLnkCache.mShpPntNum;
				os << ", but route idx " << rutidx << " link idx " << lnkidx << " shape point idx " << pntidx;
				throw std::invalid_argument(os.str());
			}
			
		}

		static void proccess(void* self)
		{
			while (MY_INVALID_HANDLE == CRouteSearchData::m_activate.m_hThread)
				MyThread_sleep_ms(100);

			auto hThd = CRouteSearchData::m_activate.m_hThread;
			if (self)
			{
				auto ins = static_cast<CRouteSearchData*>(self);
				MyMessage_t msg = { E_MID_NONE };
				const size_t tmout_ms = 1000;
				MyWaitMsgState_E ws = MY_WAIT_MSG_STATE_ERR_UNKNOWN;
				bool loop = true;

				while (loop)
				{
					ws = MyThread_waitMessage(hThd, &msg, tmout_ms);
					switch (ws)
					{
					case MY_WAIT_MSG_STATE_OK:
						switch (msg.mID)
						{
						case E_MID_STARTLOAD_JSON:
							(void)ins->procStartLoading();
							break;
						case E_MID_EXIT:
							loop = false;
							break;
						default:
							break;
						}

						break;
					case MY_WAIT_MSG_STATE_TIMEOUT:
						break;
					case MY_WAIT_MSG_STATE_INVALID:
						break;
					default:
						break;
					}
				}
			}
		}

		size_t getBranchNumber() const
		{
			return m_vecBranch.size();
		}


		void getBranchFromInfo(size_t brIdx, JsonFromInfo& fi)
		{
			if (brIdx < m_vecBranch.size())
			{
				std::tie(
					fi.m_isRevSrh,
					fi.m_isSuper,
					fi.m_dsid,
					fi.m_nilid,
					fi.m_firstShpPt,
					fi.m_lastShpPt,
					fi.m_outnum,
					m_curBrCache.m_startIdx
					) = m_vecBranch[brIdx];

				m_curBrCache.m_outnum = fi.m_outnum;
				m_curBrCache.m_brIdx = brIdx;
			}
			else
			{
				throw std::invalid_argument("branch index is overflow!");
			}
		}

		void getBranchOutInfo(size_t brIdx, size_t outIdx, JsonOutInfo& rOi)
		{
			if (brIdx == m_curBrCache.m_brIdx && outIdx < m_curBrCache.m_outnum)
			{
				std::tie(
					rOi.m_isSuper,
					rOi.m_dsid,
					rOi.m_nilid,
					rOi.m_cost
					) = m_vecOutInfo[m_curBrCache.m_startIdx + outIdx];
			}
			else
			{
				throw std::out_of_range("not the expected branch index of bad out info index");
			}
		}

	private: // prevention
		CRouteSearchData()
			: m_pfnNotifyCompleteLoading(nullptr)
			, m_iOutInfoStart(0)
			, m_iRutDsidStart(0)
			, m_curBrCache({ std::numeric_limits<size_t>::max(), 0, 0 })
			, m_curRutInfoCache({ std::numeric_limits<size_t>::max() })
		{}

		~CRouteSearchData() {}

	public: // attribute
		JsonRouteSrchDataCompleteCallbackFn_t m_pfnNotifyCompleteLoading;

	private: // data extracted from json
		std::vector<std::tuple<
			bool,	// is super link
			DSegmentId_t,	// out DSegmentID 
			NavInfoLinkId_t,	// out NavInfoId
			RouteCost_t	// out DSegment cost
			>> m_vecOutInfo;

		std::tuple<
			bool,		// is reverse search?
			bool,		// is super?
			DSegmentId_t,	// from DSegmentID
			NavInfoLinkId_t,	// from NavInfoLinkId
			JsonGeoPoint,	// first shape point
			JsonGeoPoint,	// last shape point
			size_t		// out number
		> m_fromInfo;

		std::vector<std::tuple<
			bool,		// is reverse search
			bool,		// is super of the from segment
			DSegmentId_t,	// from DSegmentID
			NavInfoLinkId_t,	// from NavInfoLinkId
			JsonGeoPoint,	// first shape point
			JsonGeoPoint,	// last shape point
			size_t,		// out number
			size_t // outinfo start index
			>> m_vecBranch;

		struct // accessor helper
		{
			size_t m_brIdx; // branch index
			size_t m_outnum; // outinf number
			size_t m_startIdx; // start index of out info
		} m_curBrCache; // cache form access

		struct {
			JsonGeoPoint mTopLeft, mBtmRight;
		} m_rectFrom, m_rectOut;

		std::vector<JsonGeoPoint> m_vecRutLnkShpPnt;
		std::vector<std::tuple<
			DSegmentId_t,
			size_t, // shape point number
			size_t // start index of shape point in vector
		>> m_vecRouteLink;
		size_t m_iRutDsidStart;	//< cursor for Json loading
		std::vector<std::tuple<
			size_t, // dsid num
			RouteCost_t, // all seg cost of route
			size_t, // traffic light number
			size_t // start index to route dsid vector
		>> m_vecRouteInfo;

		struct{ // accessor helper
			size_t mRutIdx;
			size_t mLinkNum;
			size_t m_startLinkIdx;
		}m_curRutInfoCache;

		struct{ // accessor helper
			size_t mRutIdx;
			size_t mLnkIdx;
			DSegmentId_t mDsid;
			size_t mShpPntNum;
			size_t mShpPntBasIdx;
		}m_curRutLnkCache;

		size_t m_iOutInfoStart; //< cursor for Json loading
		std::string m_strJsonPath; //< Json path
		static CActivate m_activate; // thread activator
	};

	CRouteSearchData::CActivate CRouteSearchData::m_activate(CRouteSearchData::proccess);
}




#ifdef __cplusplus
extern"C"{
#endif
	using namespace CPP11_myjson;
	void Json_setRouteSearchDataCallBack(JsonRouteSrchDataCompleteCallbackFn_t pfn)
	{
		CRouteSearchData::getInstance()->m_pfnNotifyCompleteLoading = pfn;
	}

	bool Json_asynLoadRouteSearchData(const char* path)
	{
		return CRouteSearchData::getInstance()->postStartLoading(path);
	}

	bool Json_getRouteSrchGeoInfo(JsonRouteSrchGeoInfo* pInf)
	{
		auto ins = CRouteSearchData::getInstance();
		return pInf ? (ins->getRouteSrchGeoInfo(*pInf), true) : false;
	}

	size_t Json_getRouteSrchBranchNum()
	{
		return CRouteSearchData::getInstance()->getBranchNumber();
	}

	bool Json_getSrchBrchFromInfo(size_t brIdx, JsonFromInfo* pFi)
	{
		auto ins = CRouteSearchData::getInstance();
		return pFi ? (ins->getBranchFromInfo(brIdx, *pFi), true) : false;
	}

	bool Json_getBranchOutInfo(size_t brIdx, size_t outIdx, JsonOutInfo* pOi)
	{
		auto ins = CRouteSearchData::getInstance();
		return pOi ? (ins->getBranchOutInfo(brIdx, outIdx, *pOi), true) : false;
	}

	size_t Json_getRouteNum()
	{
		return CRouteSearchData::getInstance()->getRouteNumber();
	}

	bool Json_getRouteInfo(size_t rutidx, JsonRouteInfo* pRI)
	{
		auto ins = CRouteSearchData::getInstance();
		return pRI ? (ins->getRouteInfo(rutidx, *pRI), true) : false;
	}

	bool Json_getRouteLink(size_t rutidx, size_t lnkidx, JsonRouteLink* pLnk)
	{
		auto ins = CRouteSearchData::getInstance();
		return pLnk ? (ins->getRouteLink(rutidx, lnkidx, *pLnk), true) : false;
	}

	bool Json_getRouteLinkShapePoint(size_t rutidx, size_t lnkidx, size_t pntidx, JsonGeoPoint* pPt)
	{
		auto ins = CRouteSearchData::getInstance();
		return pPt ? (ins->getRouteLinkShapePoint(rutidx, lnkidx, pntidx, *pPt), true) : false;
	}

	void Json_finalize()
	{
		CRouteSearchData::getInstance()->postExitLoop();
	}

#ifdef __cplusplus
}
#endif