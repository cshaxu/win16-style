#ifndef TEST_VB_PUMP_H
#define TEST_VB_PUMP_H

static DWORD pump(HANDLE h,DWORD timeout){
    ULONGLONG end=GetTickCount64()+timeout;
    for(;;){
        ULONGLONG now=GetTickCount64();
        if(now>=end)return WAIT_TIMEOUT;
        DWORD r=MsgWaitForMultipleObjectsEx(1,&h,(DWORD)(end-now),QS_ALLINPUT,MWMO_INPUTAVAILABLE);
        if(r!=WAIT_OBJECT_0+1)return r;
        MSG m;
        while(PeekMessageW(&m,NULL,0,0,PM_REMOVE)){
            TranslateMessage(&m);
            DispatchMessageW(&m);
        }
    }
}

#endif
