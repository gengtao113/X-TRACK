#include "dataproc_c.h"
#include "DataProc.h"

void* DataProc_Center(void)
{
    return DataProc::Center();
}

const char* DataProc_MakeTimeString(uint64_t ms, char* buf, uint16_t len)
{
    return DataProc::MakeTimeString(ms, buf, len);
}
