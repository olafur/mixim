#include "ChannelInfo.h"

#include <iostream>
#include <assert.h>

void ChannelInfo::addAirFrame(AirFrame* frame, simtime_t startTime)
{
	assert(airFrameStarts.count(frame) == 0);

	//check if we were previously empty
	if(isChannelEmpty()) {
		//earliest time point is current sim time
		earliestInfoPoint = startTime;
	}

	//calculate endTime of AirFrame
	simtime_t endTime = startTime + frame->getDuration();

	//add AirFrame to active AirFrames
	activeAirFrames[endTime].push_back(AirFrameTimePair(startTime, frame));

	//add to start time map
	airFrameStarts[frame] = startTime;

	assert(!isChannelEmpty());
}

simtime_t ChannelInfo::findEarliestInfoPoint()
{
	//TODO: check for a more efficient way to find out that earliest time-point

	// make a variable for the earliest-start-time of all remaining AirFrames
	simtime_t earliestStart = 0;
	AirFrameStartMap::const_iterator it = airFrameStarts.begin();

	// if there is an entry for an AirFrame
	if (it != airFrameStarts.end())
	{
		// store the start-time of the first entry as earliestStart so far
		earliestStart = (*it).second;

		// go through all other start-time-points
		for (; it != airFrameStarts.end(); ++it)
		{
			// and check if an earlier start-time was found,
			// if so, replace earliestStart with it
			if( (*it).second < earliestStart )
				earliestStart = (*it).second;
		}
	}

	return earliestStart;
}

simtime_t ChannelInfo::removeAirFrame(AirFrame* frame)
{
	assert(airFrameStarts.count(frame) > 0);

	//get start of AirFrame
	simtime_t startTime = airFrameStarts[frame];

	//calculate end time
	simtime_t endTime = startTime + frame->getDuration();

	//remove this AirFrame from active AirFrames
	deleteAirFrame(activeAirFrames, frame, startTime, endTime);

	//add to inactive AirFrames
	addToInactives(frame, startTime, endTime);


	// Now check, whether the earliest time-point we need to store information
	// for might have moved on in time, since an AirFrame has been deleted.
	if(isChannelEmpty()) {
		earliestInfoPoint = -1;
	} else {
		earliestInfoPoint = findEarliestInfoPoint();
	}

	return earliestInfoPoint;
}

void ChannelInfo::assertNoIntersections() {
	for(AirFrameMatrix::iterator it1 = inactiveAirFrames.begin();
		it1 != inactiveAirFrames.end(); ++it1)
	{
		simtime_t e0 = it1->first;
		for(AirFrameTimeList::iterator it2 = it1->second.begin();
			it2 != it1->second.end(); ++it2)
		{
			simtime_t s0 = it2->first;

			bool intersects = (recordStartTime > -1 && recordStartTime <= e0);

			for(AirFrameMatrix::iterator it3 = activeAirFrames.begin();
				it3 != activeAirFrames.end() && !intersects; ++it3)
			{
				simtime_t e1 = it3->first;
				for(AirFrameTimeList::iterator it4 = it3->second.begin();
					it4 != it3->second.end() && !intersects; ++it4)
				{
					simtime_t s1 = it4->first;

					if(e0 >= s1 && s0 <= e1)
						intersects = true;
				}
			}
			assert(intersects);
		}
	}
}

void ChannelInfo::deleteAirFrame(AirFrameMatrix& airFrames,
								 AirFrame* frame,
								 simtime_t startTime, simtime_t endTime)
{
	AirFrameMatrix::iterator listIt = airFrames.find(endTime);
	AirFrameTimeList* list = &listIt->second;

	for(AirFrameTimeList::iterator it = list->begin();
		it != list->end(); it++)
	{
		if(it->second == frame)
		{
			it = list->erase(it);
			if(list->empty()) {
				airFrames.erase(listIt);
			}
			return;
		}
	}

	assert(false);
}

bool ChannelInfo::canDiscardInterval(simtime_t startTime,
									 simtime_t endTime)
{
	assert(recordStartTime >= 0 || recordStartTime == -1);

	// only if it ends before the point in time we started recording or if
	// we aren't recording at all and it does not intersect with any active one
	// anymore this AirFrame can be deleted
	return (recordStartTime > endTime || recordStartTime == -1)
		   && !isIntersecting(activeAirFrames, startTime, endTime);
}

void ChannelInfo::checkAndCleanInterval(simtime_t startTime,
										 simtime_t endTime)
{


	// get through inactive AirFrame which intersected with the passed interval
	IntersectionIterator inactiveIntersectIt(&inactiveAirFrames,
											 startTime,
											 endTime);

	AirFrame* inactiveIntersect = inactiveIntersectIt.next();
	while(inactiveIntersect != 0)
	{
		simtime_t currentStart = airFrameStarts[inactiveIntersect];
		simtime_t currentEnd = currentStart + inactiveIntersect->getDuration();

		if(canDiscardInterval(currentStart, currentEnd))
		{
			inactiveIntersectIt.eraseAirFrame();

			airFrameStarts.erase(inactiveIntersect);

			delete inactiveIntersect;
			inactiveIntersect = 0;
		}
		inactiveIntersect = inactiveIntersectIt.next();
	}
}

void ChannelInfo::addToInactives(AirFrame* frame,
								 simtime_t startTime,
								 simtime_t endTime)
{
	// At first, check if some inactive AirFrames can be removed because the
	// AirFrame to in-activate was the last one they intersected with.
	checkAndCleanInterval(startTime, endTime);

	if(!canDiscardInterval(startTime, endTime))
	{
		inactiveAirFrames[endTime].push_back(AirFrameTimePair(startTime, frame));
	}
	else
	{
		airFrameStarts.erase(frame);

		delete frame;
	}
}

bool ChannelInfo::isIntersecting(const AirFrameMatrix& airFrames,
								 simtime_t from, simtime_t to) const
{
	ConstIntersectionIterator it(&airFrames, from, to);
	return (it.next() != 0);
}

void ChannelInfo::getIntersections( const AirFrameMatrix& airFrames,
									simtime_t from, simtime_t to,
									AirFrameVector& outVector) const
{
	ConstIntersectionIterator it(&airFrames, from, to);

	AirFrame* intersect = it.next();
	while(intersect != 0)
	{
		outVector.push_back(intersect);
		intersect = it.next();
	}
}

void ChannelInfo::getAirFrames(simtime_t from, simtime_t to,
							   AirFrameVector& out) const
{
	//check for intersecting inactive AirFrames
	getIntersections(inactiveAirFrames, from, to, out);

	//check for intersecting active AirFrames
	getIntersections(activeAirFrames, from, to, out);
}


