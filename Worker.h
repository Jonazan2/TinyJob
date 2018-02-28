#pragma once

#include <thread>

class JobSystem;
class JobQueue;
class Job;

class Worker
{
	public:
		enum class State : unsigned int
		{
			RUNNING = 0,
			IDLE
		};

		Worker( JobSystem *, JobQueue * );
		Worker( const Worker & ) = delete;
		~Worker();

		void StartBackgroundThread();
		void Stop();
		void Submit( Job *job );
		void Wait( Job *sentinel );
		bool IsRunning();

		const std::thread::id& GetThreadId() const;

	private:
		State state;
		JobQueue *queue;
		JobSystem *system;
		std::thread *thread;
		std::thread::id threadId;

		Job* GetJob();
		void Loop();
};