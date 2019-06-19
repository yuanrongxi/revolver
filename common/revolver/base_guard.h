#ifndef __BASE_GUARD_H
#define __BASE_GUARD_H

#include "base_namespace.h"

BASE_NAMESPACE_BEGIN_DECL
template<class BASE_MUTEX>
class BaseGuard
{
public:
	BaseGuard(BASE_MUTEX &mutex) : mutex_(mutex)
	{
		mutex_.acquire();
		locked_ = true;
	};

	~BaseGuard()
	{
		locked_ = false;
		mutex_.release();
	};

	bool locked() const 
	{
		return locked_;
	};

private:
	BASE_MUTEX&		mutex_; 
	bool			locked_;
};

BASE_NAMESPACE_END_DECL

#define BASE_GUARD_ACTION(MUTEX, OBJ, LOCK, ACTION, REACTION) \
	BaseGuard<MUTEX> OBJ(LOCK);\
	if(OBJ.locked()){ ACTION; } \
	else { REACTION; }

#define BASE_GUARD_REACTION(MUTEX, OBJ, LOCK, REACTION) \
	BASE_GUARD_ACTION(MUTEX, OBJ, LOCK, ;, REACTION)

#define BASE_GUARD(MUTEX, OBJ, LOCK) \
	BASE_GUARD_REACTION(MUTEX, OBJ, LOCK, return)

#define BASE_GUARD_RETURN(MUTEX, OBJ, LOCK, REACTION)\
	BASE_GUARD_REACTION(MUTEX, OBJ, LOCK, return REACTION)
#endif
