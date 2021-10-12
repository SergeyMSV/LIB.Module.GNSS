///////////////////////////////////////////////////////////////////////////////////////////////////
// utilsPatternState.h
//
// Standard ISO/IEC 114882, C++14
//
// |   version  |    release    | Description
// |------------|---------------|---------------------------------
// |      1     |   2020 01 07  |
// |            |               | 
///////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include <type_traits>

namespace utils
{

namespace pattern_State
{

class tObjectState
{
protected:
	class tStateBase
	{
		tObjectState* m_pObj = nullptr;

	private:
		tStateBase() { }

	public:
		explicit tStateBase(tObjectState* obj) :m_pObj(obj) { }
		virtual ~tStateBase() { }

		template <class T>
		T* GetObject() const
		{
			if (std::is_same<tObjectState, T>::value || std::is_base_of<tObjectState, T>::value)
			{
				return reinterpret_cast<T*>(m_pObj);
			}

			return nullptr;
		}

		void ChangeState(tStateBase* state) { m_pObj->ChangeState(state); }
	};

	friend class tState;

private:
	tStateBase* m_pState = nullptr;

public:
	virtual ~tObjectState()
	{
		delete m_pState;
	}

	template <class T>
	T* GetState() const
	{
		if (std::is_same<tStateBase, T>::value || std::is_base_of<tStateBase, T>::value)
		{
			return reinterpret_cast<T*>(m_pState);
		}

		return nullptr;
	}

	void ChangeState(tStateBase* state)
	{
		tStateBase* Prev = m_pState;

		m_pState = state;

		delete Prev;
	}
};

}

}
