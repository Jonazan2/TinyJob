#pragma once

#include <atomic>

using JobFunction = void(*)( void *data );

class Job 
{
	public:
		Job( JobFunction function, Job *parent, void *data );
		
		void Execute();
		void Finish();
		bool IsFinished() const;

	private:
		JobFunction function;
		Job *parent;
		std::atomic_size_t pendingJobs;
		void *data;
};