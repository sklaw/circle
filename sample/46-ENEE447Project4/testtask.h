#ifndef _testtask_h
#define _testtask_h

#include <circle/sched/task.h>
 
class CUserModeTask : public CTask
{
public:
	CUserModeTask (void);
	~CUserModeTask (void);

	void Run (void);

private:
};

#endif
