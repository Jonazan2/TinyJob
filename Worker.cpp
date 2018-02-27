#include "Worker.h"

#include <vector>
#include <thread>

#include "JobQueue.h"
#include "JobSystem.h"
#include "Job.h"

Worker::Worker( JobSystem *system, JobQueue *queue ) 
	: system( system ), queue( queue ), thread( nullptr ), threadId(std::this_thread::get_id()) {}

Worker::~Worker()
{
	Stop();
	if ( thread != nullptr )
	{
		thread->join();
		delete thread;
	}
}

void Worker::Start()
{
	state = State::RUNNING;
	thread = new std::thread( &Worker::Loop, this );
	threadId = thread->get_id();
}

void Worker::Stop()
{
	state = State::IDLE;
}

void Worker::Submit( Job *job )
{
	queue->Push( job );
}

void Worker::Wait( Job *sentinel )
{
	while ( !sentinel->IsFinished() )
	{
		Job *job = GetJob();
		if ( job != nullptr )
		{
			job->Execute();
			if ( job->IsFinished() )
			{
				delete job;
			}
		}
	}
}

void Worker::Loop()
{
	while ( IsRunning() )
	{
		Job *job = GetJob();
		if ( job != nullptr ) {
			job->Execute();
			if ( job->IsFinished() )
			{
				delete job;
			}
		}
	}
}

Job* Worker::GetJob()
{
	Job *job = queue->Pop();

	if ( job == nullptr )
	{
		JobQueue *randomQueue = system->GetRandomJobQueue();
		if ( randomQueue == nullptr )
		{
			std::this_thread::yield();
			return nullptr;
		}

		if ( queue == randomQueue )
		{
			std::this_thread::yield();
			return nullptr;
		}

		job = randomQueue->Steal();
		if ( job == nullptr )
		{
			std::this_thread::yield();
			return nullptr;
		}
	}
	return job;
}

bool Worker::IsRunning()
{
	return ( state == State::RUNNING );
}

const std::thread::id& Worker::GetThreadId() const
{
	return threadId;
}
