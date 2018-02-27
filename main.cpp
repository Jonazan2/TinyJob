#include <iostream>

#include "JobSystem.h"
#include "Worker.h"

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

	char *data = "No Job, I'm your father\n";
	Job *parent = jobSystem.CreateJob( PrintSomethingNice, data );

	for ( int i = 0; i < 50000; ++i )
	{
		Job * job = jobSystem.CreateJobAsChild( PrintSomething, parent );
		jobSystem.Run( job );
	}
	jobSystem.Run( parent );
	jobSystem.Wait( parent );
}