#include "registry.h"
static unsigned index_for(const win32_process_global_registry *r,DWORD pid,DWORD tid)
{return (unsigned)((pid^(tid*33u))%r->bucket_count);}
void win32_process_global_registry_init(win32_process_global_registry *r,win32_process_global_registry_node **b,unsigned n)
{r->buckets=b;r->bucket_count=n;r->cursor=0;}
void *win32_process_global_registry_find(const win32_process_global_registry *r,DWORD pid,DWORD tid)
{
    for(win32_process_global_registry_node *n=r->buckets[index_for(r,pid,tid)];n;n=n->next)
        if(n->pid==pid && n->tid==tid)return n->owner;
    return NULL;
}
void win32_process_global_registry_insert(win32_process_global_registry *r,win32_process_global_registry_node *n,DWORD pid,DWORD tid,void *owner)
{unsigned i=index_for(r,pid,tid);n->pid=pid;n->tid=tid;n->owner=owner;n->next=r->buckets[i];r->buckets[i]=n;}
unsigned win32_process_global_registry_reap(win32_process_global_registry *r,unsigned budget,win32_process_global_registry_remove_fn remove,win32_process_global_registry_release_fn release,void *context)
{
    unsigned removed=0;
    while(budget--){
        win32_process_global_registry_node **link=&r->buckets[r->cursor];r->cursor=(r->cursor+1)%r->bucket_count;
        while(*link){
            win32_process_global_registry_node *node=*link;void *owner=node->owner;
            if(remove(owner,context)){*link=node->next;++removed;release(owner,context);}else link=&node->next;
        }
    }
    return removed;
}
