#ifndef __STORAGE_SERVICE_C_H
#define __STORAGE_SERVICE_C_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
class StorageService;
#else
typedef struct StorageService StorageService;
#endif

#ifdef __cplusplus
extern "C" {
#endif

StorageService* StorageService_Create(const char* filePath, uint32_t bufferSize);
void            StorageService_Destroy(StorageService* s);
bool            StorageService_LoadFile(StorageService* s);
bool            StorageService_SaveFile(StorageService* s, const char* backupPath);
bool            StorageService_Add(StorageService* s, const char* key, void* value, uint16_t size, int type);
bool            StorageService_Remove(StorageService* s, const char* key);

#ifdef __cplusplus
}
#endif

#endif
