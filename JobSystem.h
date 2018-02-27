#pragma once

#include <thread>
#include <vector>
#include "Job.h"

class JobQueue;
class Worker;

class JobSystem
{
	public:
		JobSystem( size_t workersCount, size_t jobsPerWorker );
		~JobSystem();

		Job* CreateEmptyJob() const;
		Job* CreateJob( JobFunction function ) const;
		Job* CreateJob( JobFunction function, void * data ) const;
		Job* CreateJobAsChild( JobFunction function, Job *parent ) const;
		Job* CreateJobAsChild( JobFunction function, Job *parent, void *data ) const;

		void Run( Job *job );
		void Wait( Job *job );

		void ClearJobQueues();
		JobQueue* GetRandomJobQueue();
	private:
		size_t workersCount;

		std::vector< Worker * > workers;
		std::vector< JobQueue * > queues;
	
		Worker * FindWorkerWithThreadID( const std::thread::id &id ) const;
};