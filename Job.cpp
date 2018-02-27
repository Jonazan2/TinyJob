#include "Job.h"

Job::Job( JobFunction function, Job *parent, void *data ) : function( function ), parent ( parent ), data ( data ), pendingJobs { 1 } 
{
	if ( parent != nullptr )
	{
		parent->pendingJobs++;
	}
}
		
bool Job::IsFinished() const
{
	return ( pendingJobs == 0 );
}

void Job::Execute()
{
	if ( function != nullptr )
	{
		function( data );
	}
	Finish();
}

void Job::Finish()
{
	pendingJobs--;

	if ( IsFinished() && parent != nullptr )
	{
		parent->Finish();
	}
}