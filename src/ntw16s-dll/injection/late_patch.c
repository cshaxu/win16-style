/*
 *  MinHook - The Minimalistic API Hooking Library for x64/x86
 *  Copyright (C) 2009-2017 Tsuda Kageyu.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *   1. Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 *  TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 *  PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER
 *  OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 *  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 *  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 *  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 *  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 *  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/* Experimental late patch, adapted from the project startup hook backend. */
#include "late_patch.h"
#include "minhook/src/buffer.h"
#include "minhook/src/trampoline.h"
#include <stdint.h>
#include <string.h>
DWORD ntw16s_late_patch_prepare(ntw16s_late_patch *p,void *slot,void *replacement)
{
    void *target=NULL;TRAMPOLINE t={0};
    if(!p || p->target || !slot || !replacement)return ERROR_INVALID_PARAMETER;
    memcpy(&target,slot,sizeof(target));
    if(!target || !IsExecutableAddress(target) || !IsExecutableAddress(replacement))return ERROR_INVALID_ADDRESS;
    t.pTarget=target;t.pDetour=replacement;t.pTrampoline=AllocateBuffer(target);
    if(!t.pTrampoline)return ERROR_NOT_ENOUGH_MEMORY;
    if(!CreateTrampolineFunction(&t)){FreeBuffer(t.pTrampoline);return ERROR_NOT_SUPPORTED;}
    p->target=target;p->trampoline=t.pTrampoline;p->slot=slot;p->above=t.patchAbove;
#ifdef _WIN64
    p->replacement=t.pRelay;
#else
    p->replacement=t.pDetour;
#endif
    BYTE *start=(BYTE *)target-(p->above?sizeof(JMP_REL):0);
    memcpy(p->backup,start,sizeof(JMP_REL)+(p->above?sizeof(JMP_REL_SHORT):0));
    return ERROR_SUCCESS;
}
DWORD ntw16s_late_patch_install(ntw16s_late_patch *p)
{
    if(!p || !p->target || !p->trampoline || !p->slot || p->installed || p->guard.threads)return ERROR_INVALID_STATE;
    BYTE *start=(BYTE *)p->target-(p->above?sizeof(JMP_REL):0);
    SIZE_T length=sizeof(JMP_REL)+(p->above?sizeof(JMP_REL_SHORT):0);
    ntw16s_late_region region={start,length};DWORD error=ntw16s_late_guard_begin(&p->guard,&region,1);
    if(error)return error;
    if(memcmp(start,p->backup,length)){error=ERROR_REVISION_MISMATCH;goto done;}
    DWORD protection=0,ignored=0;
    if(!VirtualProtect(start,length,PAGE_EXECUTE_READWRITE,&protection)){error=GetLastError();goto done;}
    memcpy(p->slot,&p->trampoline,sizeof(p->trampoline));
    JMP_REL jump={.opcode=0xe9,.operand=(UINT32)((uintptr_t)p->replacement-(uintptr_t)(start+sizeof(JMP_REL)))};
    memcpy(start,&jump,sizeof(jump));
    if(p->above){JMP_REL_SHORT short_jump={.opcode=0xeb,.operand=(UINT8)(0-sizeof(JMP_REL)-sizeof(JMP_REL_SHORT))};memcpy(p->target,&short_jump,sizeof(short_jump));}
    p->installed=TRUE;
    if(!VirtualProtect(start,length,protection,&ignored))error=GetLastError();
    if(!FlushInstructionCache(GetCurrentProcess(),start,length) && !error)error=GetLastError();
 done:
    {DWORD release=ntw16s_late_guard_end(&p->guard);if(release && !error)error=release;}
    return error;
}
DWORD ntw16s_late_patch_discard(ntw16s_late_patch *p)
{
    if(!p || p->installed || p->guard.threads)return ERROR_INVALID_STATE;
    if(p->trampoline)FreeBuffer(p->trampoline);
    ZeroMemory(p,sizeof(*p));return ERROR_SUCCESS;
}
