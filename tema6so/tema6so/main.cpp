// ipc_counter.c
// Rulam doua instante ale programului. Am folosit un obiect mapat in memorie si un semafor
// pentru a sincroniza cresterea unui numar pana la 1000.

#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

const char* MAP_NAME = "Local\\MyCounterMap_Example12345";
const char* SEM_NAME = "Local\\MyCounterSem_Example12345";

typedef struct {
    LONG current;
    LONG finished;
} SharedData;

int main() {
    HANDLE hMap, hSem;
    SharedData* pData;
    BOOL createdMap;
    DWORD err;

    srand((unsigned int)(GetTickCount() ^ GetCurrentProcessId()));

    hMap = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE,
        0, sizeof(SharedData), MAP_NAME);
    if (!hMap) return 1;

    err = GetLastError();
    createdMap = (err != ERROR_ALREADY_EXISTS);

    pData = (SharedData*)MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(SharedData));
    if (!pData) return 1;

    if (createdMap) {
        pData->current = 0;
        pData->finished = 0;
    }

    hSem = CreateSemaphoreA(NULL, 1, 1, SEM_NAME);
    if (!hSem) return 1;

    printf("PID=%lu pornit.\n", GetCurrentProcessId());

    while (1) {
        if (pData->finished) break;

        if (WaitForSingleObject(hSem, INFINITE) != WAIT_OBJECT_0)
            break;

        LONG cur = pData->current;

        if (cur >= 1000) {
            pData->finished = 1;
            ReleaseSemaphore(hSem, 1, NULL);
            break;
        }

        printf("PID=%lu citesc %ld\n", GetCurrentProcessId(), cur);

        int coin;
        do {
            coin = (rand() % 2) + 1;
            if (coin == 2 && cur < 1000) {
                cur++;
                pData->current = cur;
                printf("PID=%lu scriu %ld\n", GetCurrentProcessId(), cur);
                Sleep(1);
            }
        } while (coin == 2 && cur < 1000);

        if (cur >= 1000)
            pData->finished = 1;

        ReleaseSemaphore(hSem, 1, NULL);
        Sleep(5);
    }

    printf("PID=%lu terminat. Final=%ld\n", GetCurrentProcessId(), pData->current);

    UnmapViewOfFile(pData);
    CloseHandle(hMap);
    CloseHandle(hSem);
    return 0;
}
