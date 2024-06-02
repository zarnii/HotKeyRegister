#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <WinUser.h>
#include <stdint.h>
#include <conio.h>
#include "bind.h"
#include "kb_button.h"
void save(void);
void start(void);

int main(void)
{
    start();

    return 0;
}



void start(void)
{
    struct BindCollection* bc = malloc(sizeof(struct BindCollection));
    uint8_t result = BindsLoad(bc);
    printf("load result: %d\n", result);

    if (result != 0)
    {
        return;
    }

    
    size_t registerResult = BindsRegister(bc);
    printf("register result: %d\n", registerResult);
    printf("%d\n", bc->binds[0]->Id);

    if (registerResult != 0)
    {
        return;
    }
    
    /*
        Проблема в том, что функция GetMessage
        слушает сообщения в вызывающем потоке.
        Горячие клавиши регистрируются в основном потоке, а GetMessage
        запускается в фоновом.

        Вариант: регистрировать горячие клавиши и сразу же запускать GetMessage в фоновом потоке.
    */
    printf("start listen...");
    HANDLE hThread;
    BindsStartListen(hThread);

    WaitForSingleObject(hThread, INFINITE);
    
}

void save(void)
{
    char* name = "F:/C++/hello/main.exe";
    char* name2 = "F:/Python/test.py";
    
    struct BindCollection* bc = malloc(sizeof(struct BindCollection));
    bc->binds = malloc(PBIND_SIZE * 2);

    struct Bind* bind1 = malloc(BIND_SIZE);
    struct BindData bindData1;
    bindData1.PathToFile = name;
    bindData1.StrSize = strlen(name);
    bind1->Id = 1;
    bind1->CommandKey = MOD_CONTROL;
    bind1->VirtualKey = BUTTON_W;
    bind1->Act = ExecuteFile;
    bind1->Data = bindData1;

    struct Bind* bind2 = malloc(BIND_SIZE);
    struct BindData bindData2;
    bindData2.PathToFile = name2;
    bindData2.StrSize = strlen(name2);
    bind2->Id = 2;
    bind2->CommandKey = MOD_CONTROL;
    bind2->VirtualKey = BUTTON_Q;
    bind2->Act = ExecutePythonScript;
    bind2->Data = bindData2;

    bc->binds[0] = bind1;
    bc->binds[1] = bind2;
    bc->count = 2;
    
    uint8_t result = BindsSave(bc);
    printf("save result: %d", result);
}