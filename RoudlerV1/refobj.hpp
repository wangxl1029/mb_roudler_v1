#pragma once


namespace nsRefobj
{
	typedef void* Handle_t;
	const Handle_t InvalidReference = NULL;

	class CReferenceBase
	{
		struct CRefInfo
		{
			size_t mCount;
			CReferenceBase* pMe;
			Handle_t hMe;
		};
	protected:
		CReferenceBase() : m_bAutoDelete(true)
		{
			m_ref.mCount = 0;
			m_ref.pMe = this;
			m_ref.hMe = &m_ref;
		}

		CReferenceBase(const CReferenceBase&) = delete;
		CReferenceBase& operator=(const CReferenceBase&) = delete;
		virtual ~CReferenceBase() {}
		virtual void Destroy()
		{
			delete this;
		}

		virtual Handle_t Refer()
		{
			m_ref.mCount++;
			return m_ref.hMe;
		}
	public:
		static CReferenceBase* Refer(Handle_t h)
		{
			ASSERT_POINTER(h, CRefInfo);
			CRefInfo* pInfo = static_cast<CRefInfo*>(h);
			pInfo->mCount++;
			return pInfo->pMe;
		}

		static bool Release(Handle_t h)
		{
			ASSERT_POINTER(h, CRefInfo);
			CRefInfo* pInfo = static_cast<CRefInfo*>(h);
			return pInfo->pMe->Release();
		}

		bool Release()
		{
			size_t oldcnt = (m_ref.mCount > 0) ? m_ref.mCount-- : 0;

			if (0 == m_ref.mCount && m_bAutoDelete)
			{
				Destroy();
			}

			return oldcnt != m_ref.mCount;
		}

		size_t GetRefCount() const
		{
			return m_ref.mCount;
		}
		const bool m_bAutoDelete;
	private:
		CRefInfo m_ref;
	};

	template<typename refderive_cls>
	refderive_cls* Refer(Handle_t h)
	{
		return dynamic_cast<refderive_cls*>(CReferenceBase::Refer(h));
	}

	inline bool Discard(Handle_t h)
	{
		return CReferenceBase::Release(h);
	}

	inline bool IsValid(Handle_t h)
	{
		return InvalidReference != h;
	}

	inline bool IsMultiRefered(Handle_t h)
	{
		CReferenceBase* pRef = CReferenceBase::Refer(h);
		size_t refcnt = pRef->GetRefCount();
		pRef->Release();
		return refcnt > 2;
	}
};