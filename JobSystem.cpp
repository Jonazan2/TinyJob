#include "JobSystem.h"

#include <random>
#include <thread>

#include "JobQueue.h"
#include "Worker.h"

JobSystem::JobSystem( size_t workersCount, size_t jobsPerWorker) : workersCount( workersCount )
{
	queues.reserve( workersCount );
	workers.reserve( workersCount );

	// Add a main thread worker
	JobQueue *queue = new JobQueue( jobsPerWorker );
	queues.push_back( queue );
	Worker *mainThreadWorker = new Worker( this, queue );
	workers.push_back( mainThreadWorker );

	// Add the rest of the workers
	for ( size_t i = 0; i < workersCount; ++i )
	{
		JobQueue *queue = new JobQueue( jobsPerWorker );
		queues.push_back( queue );
		Worker *worker = new Worker( this, queue );
		workers.push_back( worker );
	}

	// Start background thread for all the workers except the main thread one (first on the vector)
	for ( size_t i = 1; i <= workersCount; ++i )
	{
		workers[i]->StartBackgroundThread();
	}
}

JobSystem::~JobSystem()
{	
	for ( Worker *worker : workers)
	{
		delete worker;
	}
	workers.clear();

	for ( JobQueue *queue : queues )
	{
		delete queue;
	}
	queues.clear();
}

Job* JobSystem::CreateEmptyJob() const
{
	return CreateJob( nullptr, nullptr );
}

Job* JobSystem::CreateJob( JobFunction function ) const
{
	return CreateJob( function, nullptr );
}

Job* JobSystem::CreateJob( JobFunction function, void *data ) const
{
	// TODO (jonathan): Change this for a pool of jobs
	return new Job( function, nullptr, data );
}

Job* JobSystem::CreateJobAsChild( JobFunction function, Job *parent ) const
{
	return CreateJobAsChild( function, parent, nullptr );
}

Job* JobSystem::CreateJobAsChild( JobFunction function, Job *parent, void *data ) const
{
	// TODO (jonathan): Change this for a pool of jobs
	return new Job( function, parent, data );
}

void JobSystem::Run( Job *job )
{
	Worker * worker = FindWorkerWithThreadID( std::this_thread::get_id() );
	if ( worker != nullptr )
	{
		worker->Submit( job );
	}
}

void JobSystem::Wait( Job *job )
{
	Worker * worker = FindWorkerWithThreadID( std::this_thread::get_id() );
	if ( worker != nullptr )
	{
		worker->Wait( job );
	}
}

JobQueue* JobSystem::GetRandomJobQueue()
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> distribution( 0, workersCount );

	size_t index = static_cast<size_t> ( std::round( distribution( gen ) ) );
	return queues[ index ];
}

Worker * JobSystem::FindWorkerWithThreadID( const std::thread::id &id ) const
{
	for ( Worker *worker : workers )
	{
		if ( id == worker->GetThreadId() )
		{
			return worker;
		}
	}
	return nullptr;
}

void JobSystem::ClearJobQueues()
{
	for ( JobQueue *queue : queues )
	{
		queue->Clear();
	}
}