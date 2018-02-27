#pragma once

#include <atomic>
#include <vector>

#include "Job.h"

class JobQueue {
public:
	// maxJobs must be a power of 2 for the circular queue to work properly
	JobQueue( size_t maxJobs );

	void Push( Job *job );
	Job* Pop();
	Job* Steal();

	size_t Size() const;
	void Clear();

private:
	int maxJobs;
	unsigned int mask;

	std::atomic_int bottomIndex;
	std::atomic_int topIndex;
	std::vector< Job * > queue;
};
