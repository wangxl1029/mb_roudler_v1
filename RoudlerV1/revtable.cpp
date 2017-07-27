#include "stdafx.h"
#include "revtable.hpp"

namespace nsDocHidden
{
	bool CRevTable::BuildQuery()
	{
		FromQueryInfo_map& rmpFrom = m_mapFromQInfo; // refer to from map constainer
		ParentBranchQueryInfo_map& rmpParent = m_mapParentQInfo; // refer to parent map container
		if (!rmpFrom.IsEmpty())
		{
			rmpFrom.RemoveAll();
		}

		if (!rmpFrom.IsEmpty())
		{
			CleanParentQuery();
		}

		JsonFromInfo jFrom;
		JsonOutInfo jOut;
		CFromQInfo fqi;
		QParentBranchEle_t* pPrtIdxList;
		CParentBranchIndex ParentIdx;
		size_t branchnum = Json_getRouteSrchBranchNum();
		for (size_t i = 0; i < branchnum; i++)
		{
			if (Json_getSrchBrchFromInfo(i, &jFrom))
			{
				fqi.mIdx = i;
				fqi.mRepcnt = 0;
				if (rmpFrom.Lookup(jFrom.m_dsid, fqi))
				{
					fqi.mRepcnt++;
				}
				rmpFrom.SetAt(jFrom.m_dsid, fqi);
			}

			for (size_t j = 0; j < jFrom.m_outnum; j++)
			{
				Json_getBranchOutInfo(i, j, &jOut);
				ParentIdx.mBranchIdx = i;
				ParentIdx.mOutIdx = j;

				if (rmpParent.Lookup(jOut.m_dsid, pPrtIdxList))
				{
					// do nothing, simply retrieve the parent list
				}
				else
				{
					pPrtIdxList = new QParentBranchEle_t;
				}

				pPrtIdxList->AddHead(ParentIdx);
				rmpParent.SetAt(jOut.m_dsid, pPrtIdxList);
			}
		}

		return !rmpFrom.IsEmpty() || !rmpParent.IsEmpty();
	}

}