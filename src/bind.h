#ifndef __BIND_H__
#define __BIND_H__

#include <stdint.h>
#include <Windows.h>
#include <fileapi.h>
#include <minwindef.h>
#include <string.h>
#include "action.h"

#define DEFAULT_FILE_NAME "F:/C++/HotKeyRegister/bin/binds.bin"
#define ERROR_FILE_OPEN 1
#define ERROR_FILE_WRITE 2
#define ERROR_FILE_READ 3
#define ERROR_MALLOC 4
#define ERROR_BIND_REMOVE 5
#define BIND_SIZE sizeof(struct Bind)
#define PBIND_SIZE sizeof(struct Bind*)
#define BIND_DATA_SIZE sizeof(struct BindData)
#define DEFAULT_BIND_COLLECTION_COPASITY 10

struct BindData
{
    LPSTR PathToFile;
    size_t StrSize;
};

struct Bind
{
    /* Id бинда. */
    size_t Id;

    /* Id  командной клавиши (ALT, CTRL, SHIFT). */
    UINT CommandKey;

    /* Id виртуальной клавиши. */
    UINT VirtualKey;

    /* Действие */
    enum Action Act;

    /* Дополнительная информация. */
    struct BindData Data;
};

struct BindCollection
{
    struct Bind** binds;
    size_t count;
    size_t copasity;
};

uint8_t BindsLoad(struct BindCollection* bc);
uint8_t BindsSave(struct BindCollection* bc);
uint8_t BindsAdd(struct BindCollection* bc, struct Bind* bind);
uint8_t BindsRemove(struct BindCollection* bc, size_t index);
size_t BindsRegister(struct BindCollection* bc);
void BindsStartListen(HANDLE hThread);
void BindsStopListen(HANDLE hThread);

#endif