#include "bind.h"
#include <stdio.h>
uint8_t SaveBindCount(HANDLE hFile, size_t bindCount);
uint8_t SaveBind(HANDLE hFile, struct Bind* bind, size_t size);
uint8_t SaveBindData(HANDLE hFile, struct BindData* bd);
uint8_t LoadBindCount(HANDLE hFile, size_t* bindCount);
uint8_t LoadBind(HANDLE hFile, struct Bind* bind, size_t bindSize);
uint8_t LoadBindData(HANDLE hFile, struct BindData* bindData);
uint8_t ResizeBindCollection(struct BindCollection* bc, size_t newCopasity);
void FreeBind(struct Bind* bind);
uint8_t CopyBindArrayStartIndex(struct Bind** receiver, struct Bind** source, size_t count, size_t startIndex);
void PtrSwap(void** ptr1, void** ptr2);
void MoveToEnd(struct Bind** binds, size_t length, size_t index);
DWORD WINAPI Loop(LPVOID lpParam);

size_t BindsRegister(struct BindCollection* bc)
{
    size_t errorCount = 0;
    for (size_t i = 0; i < bc->count; i++)
    {
        BOOL registerResult = RegisterHotKey(NULL, bc->binds[i]->Id, bc->binds[i]->CommandKey, bc->binds[i]->VirtualKey);

        if (!registerResult)
        {
            printf("ERROR: register hot key id: %d", bc->binds[i]->Id);
            errorCount++;
        }
    }

    return errorCount;
}

void BindsStartListen(HANDLE hThread)
{
    hThread = CreateThread(NULL, 0, Loop, NULL, 0, NULL);
}

void BindsStopListen(HANDLE hThread)
{
    CloseHandle(hThread);
}

uint8_t BindsLoad(struct BindCollection* bc)
{
    HANDLE hFile = CreateFile(DEFAULT_FILE_NAME, 
        GENERIC_READ,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (hFile == INVALID_HANDLE_VALUE)
    {
        return ERROR_FILE_OPEN;
    }

    size_t bindCount = 0;
    uint8_t loadBindCount = LoadBindCount(hFile, &bindCount);

    if (loadBindCount != 0)
    {
        CloseHandle(hFile);
        return loadBindCount;
    }

    if (bindCount > DEFAULT_BIND_COLLECTION_COPASITY)
    {
        bc->copasity = bindCount;
    }
    else
    {
        bc->copasity = DEFAULT_BIND_COLLECTION_COPASITY;
    }

    bc->count = bindCount;
    bc->binds = malloc(PBIND_SIZE * bc->copasity);

    for (size_t i = 0; i < bindCount; i++)
    {
        bc->binds[i] = malloc(BIND_SIZE);
        uint8_t loadBind = LoadBind(hFile, bc->binds[i], BIND_SIZE);

        if (loadBind != 0)
        {
            CloseHandle(hFile);
            return loadBind;
        }

        bc->binds[i]->Data.PathToFile = malloc(bc->binds[i]->Data.StrSize);
        uint8_t loadBindData = LoadBindData(hFile, &bc->binds[i]->Data);

        if (loadBindCount != 0)
        {
            CloseHandle(hFile);
            return loadBindData;
        }
    }

    CloseHandle(hFile);


    return 0;
}

uint8_t BindsSave(struct BindCollection* bc)
{
    HANDLE hFile = CreateFile(DEFAULT_FILE_NAME,
        GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (hFile == INVALID_HANDLE_VALUE)
    {
        return ERROR_FILE_OPEN;
    }

    uint8_t saveBindCount = SaveBindCount(hFile, bc->count);

    if (saveBindCount != 0)
    {
        CloseHandle(hFile);
        return saveBindCount;
    }

    for (size_t i = 0; i < bc->count; i++)
    {
        uint8_t saveBind = SaveBind(hFile, bc->binds[i], BIND_SIZE);

        if (saveBind != 0)
        {
            CloseHandle(hFile);
            return saveBind;
        }

        uint8_t saveBindData = SaveBindData(hFile, &bc->binds[i]->Data);

        if (saveBindData != 0)
        {
            CloseHandle(hFile);
            return saveBindData;
        }
    }

    CloseHandle(hFile);

    return 0;
}

uint8_t BindsAdd(struct BindCollection* bc, struct Bind* bind)
{
    if (bc->count == bc->copasity)
    {
        uint8_t resizeResult = ResizeBindCollection(bc, bc->copasity*2);
        if (resizeResult == ERROR_MALLOC)
        {
            return ERROR_MALLOC;
        }
    }

    bc->binds[bc->count] = malloc(PBIND_SIZE);
    bc->binds[bc->count] = bind;
    bc->count++;

    return 0;
}

uint8_t BindsRemove(struct BindCollection* bc, size_t index)
{
    if (bc->count == 0)
    {
        return ERROR_BIND_REMOVE;
    }

    MoveToEnd(bc->binds, bc->count, index);
    FreeBind(bc->binds[--bc->count]);

    return 0;
}

uint8_t SaveBindCount(HANDLE hFile, size_t bindCount)
{
    DWORD byteWritten = 0;
    BOOL errorFlag = WriteFile(hFile, &bindCount, sizeof(size_t), &byteWritten, NULL);

    if (!errorFlag)
    {
        return ERROR_FILE_WRITE;
    }

    if (byteWritten != sizeof(size_t))
    {
        return ERROR_FILE_WRITE;
    }

    return 0;
}

uint8_t SaveBind(HANDLE hFile, struct Bind* bind, size_t size)
{
    DWORD byteWritten = 0;
    BOOL errorFlag = WriteFile(hFile, bind, size, &byteWritten, NULL);

    if (!errorFlag)
    {
        return ERROR_FILE_WRITE;
    }

    if (byteWritten != size)
    {
        return ERROR_FILE_WRITE;
    }

    return 0;
}

uint8_t SaveBindData(HANDLE hFile, struct BindData* bd)
{
    DWORD byteWritten = 0;
    BOOL errorFlag = WriteFile(hFile, bd->PathToFile, bd->StrSize, &byteWritten, NULL);

    if (!errorFlag)
    {
        return ERROR_FILE_WRITE;
    }

    if (byteWritten != bd->StrSize)
    {
        return ERROR_FILE_WRITE;
    }

    return 0;
}

uint8_t LoadBindCount(HANDLE hFile, size_t* bindCount)
{
    DWORD byteReader = 0;
    BOOL errorFlag = ReadFile(hFile, bindCount, sizeof(size_t), &byteReader, NULL);

    if (!errorFlag)
    {
        return ERROR_FILE_READ;
    }

    if (byteReader != sizeof(size_t))
    {
        return ERROR_FILE_READ;
    }

    return 0;
}

uint8_t LoadBind(HANDLE hFile, struct Bind* bind, size_t bindSize)
{
    DWORD byteReader = 0;
    BOOL errorFlag = ReadFile(hFile, bind, bindSize, &byteReader, NULL);

    if (!errorFlag)
    {
        return ERROR_FILE_READ;
    }

    if (byteReader != bindSize)
    {
        return ERROR_FILE_READ;
    }

    return 0;
}

uint8_t LoadBindData(HANDLE hFile, struct BindData* bindData)
{
    DWORD byteReader = 0;
    BOOL errorFile = ReadFile(hFile, bindData->PathToFile, bindData->StrSize, &byteReader, NULL);

    if (!errorFile)
    {
        return ERROR_FILE_READ;
    }

    if (byteReader != bindData->StrSize)
    {
        return ERROR_FILE_READ;
    }

    return 0;
}

uint8_t ResizeBindCollection(struct BindCollection* bc, size_t newCopasity)
{
    struct Bind** newCollection = malloc(PBIND_SIZE * newCopasity);

    if (newCollection == NULL)
    {
        return ERROR_MALLOC;
    }

    for (size_t i = 0; i < bc->count; i++)
    {
        newCollection[i] = bc->binds[i];
    }

    free(bc->binds);
    bc->binds = newCollection;
    bc->copasity = newCopasity;

    return 0;
}

uint8_t CopyBindArrayStartIndex(struct Bind** receiver, struct Bind** source, size_t count, size_t startIndex)
{
    for (size_t i = startIndex; i < count; i++)
    {
        struct Bind* bind = source[i + 1];
        receiver[i] = source[i + 1];
    }

    return 0;
}

void FreeBind(struct Bind* bind)
{
    if (*bind->Data.PathToFile != '\0')
    {
        free(bind->Data.PathToFile);
    }
    
    free(bind);
}

void MoveToEnd(struct Bind** binds, size_t length, size_t index)
{
    for (size_t i = index; i < length - 1; i++)
    {
        PtrSwap((void*)&binds[i], (void*)&binds[i + 1]);
    }
}

void PtrSwap(void** ptr1, void** ptr2)
{
    void* temp = *ptr1;
    *ptr1 = *ptr2;
    *ptr2 = temp;
    temp = NULL;
}

DWORD WINAPI Loop(LPVOID lpParam)
{
    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0) != 0)
    {
        if (msg.message == WM_HOTKEY)
        {
            UINT_PTR hotKeyId = (UINT_PTR)msg.wParam;
            printf("hot key id = %u\n", hotKeyId);
        }
    }
    
}