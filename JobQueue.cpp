#include "JobQueue.h"

#include <algorithm>

JobQueue::JobQueue( size_t maxJobs ) 
	: maxJobs ( maxJobs )
{
	bottomIndex = 0;
	topIndex = 0;
	queue.resize( maxJobs );
}

void JobQueue::Push( Job *job )
{
	int bottom = bottomIndex.load( std::memory_order_acquire );
	queue[ bottom ] = job;
	bottomIndex.store( bottom + 1, std::memory_order_release );
}

Job* JobQueue::Pop()
{
	int bottom = bottomIndex.load( std::memory_order_acquire );
	bottom = std::max ( 0, bottom - 1 );
	bottomIndex.store( bottom, std::memory_order_release );
    int top = topIndex.load( std::memory_order_acquire );

	if ( top <= bottom )
	{
		Job *job = queue[ bottom ];

		// There are several jobs in the queue, we don't need to worry about Steal()
		if ( top != bottom )
		{
			return job;
		}

		// This is the last item in the queue, we need to check if a Steal() has increased top
		int stolenTop = top + 1;
		if( topIndex.compare_exchange_strong( stolenTop, top + 1, std::memory_order_acq_rel ) )
		{
			// An Steal() call has stolen our job (https://www.youtube.com/watch?v=DEiWU1MbBfk)
			bottomIndex.store( stolenTop, std::memory_order_release );
			return nullptr;
		}
		return job;
	}
	else
	{
		bottomIndex.store( top, std::memory_order_release );
		return nullptr;
	}
}

Job* JobQueue::Steal()
{
	int top = topIndex.load( std::memory_order_acquire );
	int bottom = bottomIndex.load( std::memory_order_acquire );

	if ( topIndex < bottomIndex )
	{
		Job *job = queue[ top ];

		// Check if a Pop() or another Steal() operation has stolen our job
		if( topIndex.compare_exchange_strong( top, top + 1, std::memory_order_acq_rel) )
		{
			return job;
		}

		return nullptr;
	}
	else
	{
		// nothing left to steal
		return nullptr;
	}
}

size_t JobQueue::Size() const
{
	return bottomIndex - topIndex;
}

void JobQueue::Clear()
{
	bottomIndex = 0;
	topIndex = 0;
}