#include <iostream>

#include <string>
#include "JobSystem.h"

void PrintSomethingNice( void *data )
{
	std::cout << (char*)data;
}

void PrintSomething( void *data )
{
	std::cout << "You killed my father!\n";
}

int main(int argc, char **argv)
{
	JobSystem jobSystem( 7, 65536 );

	/* Example of how to use a job as a parent to create a fence */
	std::string data = "No Job, I'm your father\n";
	Job* parent = jobSystem.CreateJob(PrintSomethingNice, (void*) data.c_str() );

	for ( int i = 0; i < 10000; ++i )
	{
		Job *job = jobSystem.CreateJobAsChild( PrintSomething, parent );
		jobSystem.Run( job );
	}
	jobSystem.Run( parent );
	jobSystem.Wait( parent );
}