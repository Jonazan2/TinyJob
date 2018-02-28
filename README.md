
# TinyJob
TinyJob is a simple job system implemented in C++11. It is based on the execution of several workers in parallel that are feeded a job via a free-lock stealing queue. The system also supports hierarchy so we can wait in the main thread until all the jobs that are pending from a root one get executed before continuing. 

## Workers
The system contains N workers and a job queue per worker. Workers are in charge of executing jobs or wait until a job has been completed. Jobs can only be added to the worker running in the current thread. There will always be a worker on the main thread.

Workers get the jobs that they must execute via a method called GetJob(). This method will try first to Pop() a job from the workers' own queue and if there is no job left to be executed will try to Steal() a Job from the queue of another worker. By doing this, workers are always running and tasks are distributed via Steal() operations.

## Free lock job stealing queue
The key of the whole system is the implementation of a lock-free stealing queue. By lock-free we mean that there is no lock system, like mutexes or spin locks, in place to coordinate different threads. In order to achieve this we must use atomic variables and compare and swap operations.

The implementation of the JobQueue is based on two atomic integers: top and bottom. Each queue provides three main functionalities: Push(), Pop() and Steal(). The first two operations: Push() and Pop() must always be executed by the same thread and never concurrently, the last one on the other hand must only be executed concurrently.

<p align="center">
  <img src="http://jonathanmcontreras.com/images/portfolio/tinyjob/initial.png" alt="initial state"/>
</p>

Each worker will Push() elements to its own queue and Pop() elements from its own queue in a LIFO way. Other workers will request a Steal() operation using a FIFO strategy. This means that the queue behaves like a FIFO and a LIFO at the same time depending of the operation that we are calling.

### Push
This operation adds a new element into the queue. It only increments the bottom index and doesn't need to perform any CAS operation.

<p align="center">
  <img src="http://jonathanmcontreras.com/images/portfolio/tinyjob/push.png" alt="push one element"/>
</p>

<p align="center">
  <img src="http://jonathanmcontreras.com/images/portfolio/tinyjob/push_2.png" alt="push several elements"/>
</p>

### Pop
This operation extracts and element from the queue. It only decrements the bottom index. In case there is only an element left in the queue it must execute a CAS operation with the top index value to check if a Steal() operation has already taken away the last job before we could complete the Pop().
<p align="center">
  <img src="http://jonathanmcontreras.com/images/portfolio/tinyjob/pop.png" alt="pop"/>
</p>

### Steal
Other concurrent workers may request via GetJob() a Steal() operation. This operation extracts and element from the queue. It only increments the top index. In case there is only an element left in the queue it must execute a CAS operation with the bottom index value to check if a Pop() operation has already taken away the last job before we could complete the Steal().
<p align="center">
  <img src="http://jonathanmcontreras.com/images/portfolio/tinyjob/steal.png" alt="steal"/>
</p>


### Synchronizing without locks
If we would synchronize our queues with mutexes most of our background workers will be constantly put to sleep and awaken. This operation is costly and in performance sensitive systems we can't afford that. We could also try to use spin locks but they are still more expensive that implementing a queue that is lock free.

The main point of the queue is that Push() and Pop() only touch the bottom index and Steal() only touches the top index. By implementing the queue like this we only need to execute a CAS operation when there is only one element left in the queue to ensure that Steal() hasn't stolen a job while we were doing a Pop() and viceversa, that a Pop() hasn't happened before we completed a Steal() and we don't need to synchronize any other case.

It is also crucial that in the Push() and Pop() operations the variable bottom index always gets read before top index and in the Steal() operation the variable top gets read before the bottom one. In order to assure that no compiler optimization may change the order of the read or write sentences we must add compiler barriers.

```c++
void JobQueue::Push( Job *job )
{
	int bottom = bottomIndex.load( std::memory_order_acquire );
	queue[ bottom ] = job;
	bottomIndex.store( bottom + 1, std::memory_order_release );
}
```

Compiler barriers are implemented via the API offered by the atomic library in C++11: http://en.cppreference.com/w/cpp/atomic/atomic

## Adding jobs to the system

Jobs are created and added via the JobSystem class. In order to add a job to the system we must simply call Run(). At that moment the thread worker will submit that job to its queue.

```c++
Job *job = jobSystem.CreateJob( function , data );
jobSystem.Run( job );
```

If we want the thread worker to wait until a certain job has finished its execution we must call Wait(). Our parent job only acts as a fence, and it may be executed before its children.

```c++
Job *parent = jobSystem.CreateEmptyJob();

for ( int i = 0; i < 10; ++i )
{
  Job *job = jobSystem.CreateJobAsChild( another_function, parent );
  jobSystem.Run( job );
}
jobSystem.Run( parent );
jobSystem.Wait( parent );
```


## Use cases
The system is design with the idea that the main thread will create a high level job and wait until it finishes. This high level job will maybe create more jobs to complete its execution and it will add them to the system via the worker in which is being executed. 

You can for example create a high level job for creating a save file that will then create several jobs, one per sub systems, each of them saving the information of their own parts. You can as well create a job that will trigger a request and put that background worker to wait until the request is completed.

## Possible improvements
The system right now is quite simple and there are some obvious improvements that can be made: 
* Right now jobs can only receive data via objects in the Heap. It would be nice to give more versatility to the system in this regard.
* It would also be quite nice to add a dependency system between jobs so one job will be executed before other if we want so.

## Documentation
There are a few great articles regarding job systems and lock-free programming. TinyJob is based on the articles from molecular matters: https://blog.molecular-matters.com that talk about their job system and you may find a nice introduction of lock-free programing in the Preshing on Programming blog: http://preshing.com/20120612/an-introduction-to-lock-free-programming/.
