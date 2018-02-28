#pragma once

#include <atomic>
#include <vector>

#include "Job.h"

class JobQueue {
public:
	JobQueue( size_t maxJobs );

	void Push( Job *job );
	Job* Pop();
	Job* Steal();

	size_t Size() const;
	void Clear();

private:
	int maxJobs;

	std::atomic_int bottomIndex;
	std::atomic_int topIndex;
	std::vector< Job * > queue;
};
