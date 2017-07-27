#pragma once
#include "refobj.hpp"
#include "myjson.h"

namespace nsDocHidden
{
	class CRevTable : public nsRefobj::CReferenceBase
	{
	public:
		struct CFromQInfo // query inforation
		{
			size_t mIdx; // from info index
			size_t mRepcnt; // conflick number, zero means no conflict
		};

		struct CParentBranchIndex
		{
			size_t mBranchIdx; // parent branch index
			size_t mOutIdx; // out link index at parent branch
		};

		typedef CList<CParentBranchIndex> QParentBranchEle_t; // branch typed element for query
		typedef CMap<DSegmentId_t, DSegmentId_t, QParentBranchEle_t*, QParentBranchEle_t*> ParentBranchQueryInfo_map;
		typedef CMap<DSegmentId_t, DSegmentId_t, CFromQInfo, CFromQInfo&> FromQueryInfo_map;

		static nsRefobj::Handle_t CreateReference()
		{
			CRevTable* pMe = new CRevTable;
			return pMe->Refer();
		}

		bool BuildQuery();

		size_t GetBranchInfoNum() const
		{
			return m_mapFromQInfo.GetCount();
		}

		size_t GetParentInfoNum() const
		{
			return m_mapParentQInfo.GetCount();
		}

		bool QueryBranchInfo(DSegmentId_t dsid, CFromQInfo& rInf) const
		{
			return !!m_mapFromQInfo.Lookup(dsid, rInf);
		}

		bool QueryParentBranchInfo(DSegmentId_t dsid, QParentBranchEle_t*& rpPrtLst) const
		{
			return !!m_mapParentQInfo.Lookup(dsid, rpPrtLst);
		}
	private:
		CRevTable() // not allow ctor() from outside
		{}

		~CRevTable() final // not allow new operator
		{
			CleanParentQuery();
			TRACE("reversion query table dtor()\n");
		}

		void CleanParentQuery()
		{
			DSegmentId_t dsid; // place holder
			QParentBranchEle_t* pParent; 
			ParentBranchQueryInfo_map& rmp = m_mapParentQInfo; // refer to parent map container
			POSITION pos = rmp.GetStartPosition();
			while (pos != NULL)
			{
				rmp.GetNextAssoc(pos, dsid, pParent);
				delete pParent;
			}

			if (!rmp.IsEmpty())
			{
				rmp.RemoveAll();
			}
		}
	private:
		FromQueryInfo_map m_mapFromQInfo;
		ParentBranchQueryInfo_map m_mapParentQInfo;
	};
};

