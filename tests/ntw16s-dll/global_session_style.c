#include "ntw16s-dll/global/session.h"
#include <stddef.h>
#include <stdio.h>

int main(void)
{
    ntw16s_global_session session={0};DWORD low=0,high=0;
    if(!win32_process_global_birth(GetCurrentProcess(),&low,&high))return 2;
    session.magic=NTW16S_GLOBAL_MAGIC;session.version=NTW16S_GLOBAL_VERSION;
    session.owner_pid=GetCurrentProcessId();session.owner_birth_low=low;session.owner_birth_high=high;
    session.cookie=123;session.phase=NTW16S_GLOBAL_ACTIVE;session.style=NTW16S_GLOBAL_STYLE_16;
    BOOL live16=ntw16s_global_owner_alive(&session);
    BOOL match16=ntw16s_global_session_matches(&session,123,NTW16S_GLOBAL_STYLE_16) &&
                 !ntw16s_global_session_matches(&session,123,NTW16S_GLOBAL_STYLE_2K);
    session.style=NTW16S_GLOBAL_STYLE_2K;
    BOOL live2k=ntw16s_global_owner_alive(&session);
    BOOL match2k=ntw16s_global_session_matches(&session,123,NTW16S_GLOBAL_STYLE_2K) &&
                 !ntw16s_global_session_matches(&session,123,NTW16S_GLOBAL_STYLE_16) &&
                 !ntw16s_global_session_matches(&session,124,NTW16S_GLOBAL_STYLE_2K);
    session.version=1;BOOL old_rejected=!ntw16s_global_owner_alive(&session);
    printf("GLOBAL SESSION STYLE bits=%u size=%u offset=%u live16=%d live2k=%d match16=%d match2k=%d old_rejected=%d\n",
           (unsigned)(sizeof(void *)*8),(unsigned)sizeof(session),(unsigned)offsetof(ntw16s_global_session,style),
           live16,live2k,match16,match2k,old_rejected);
    return sizeof(session)==40 && offsetof(ntw16s_global_session,style)==36 && live16 && live2k && match16 && match2k && old_rejected?0:3;
}
