#include "storage_service_c.h"
#include "StorageService.h"

StorageService* StorageService_Create(const char* filePath, uint32_t bufferSize)
{
    return new StorageService(filePath, bufferSize);
}

void StorageService_Destroy(StorageService* s)
{
    delete s;
}

bool StorageService_LoadFile(StorageService* s)
{
    return s->LoadFile();
}

bool StorageService_SaveFile(StorageService* s, const char* backupPath)
{
    return backupPath ? s->SaveFile(backupPath) : s->SaveFile();
}

bool StorageService_Add(StorageService* s, const char* key, void* value, uint16_t size, int type)
{
    return s->Add(key, value, size, (StorageService::DataType_t)type);
}

bool StorageService_Remove(StorageService* s, const char* key)
{
    return s->Remove(key);
}
