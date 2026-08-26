#include "track_point_filter_c.h"
#include "TrackPointFilter.h"

TrackPointFilter* TrackPointFilter_Create(void)
{
    return new TrackPointFilter();
}

void TrackPointFilter_Destroy(TrackPointFilter* f)
{
    delete f;
}

void TrackPointFilter_Reset(TrackPointFilter* f)
{
    f->Reset();
}

bool TrackPointFilter_PushPoint(TrackPointFilter* f, double x, double y)
{
    return f->PushPoint(x, y);
}

void TrackPointFilter_SetOffsetThreshold(TrackPointFilter* f, double offset)
{
    f->SetOffsetThreshold(offset);
}

void TrackPointFilter_GetCounts(TrackPointFilter* f, uint32_t* sum, uint32_t* output)
{
    f->GetCounts(sum, output);
}
