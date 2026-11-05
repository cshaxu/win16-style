#include "queue.h"
#include <stddef.h>
void win32_process_global_queue_push(win32_process_global_queue *queue,win32_process_global_queue_node *node)
{
    node->next=NULL;
    if(queue->tail)queue->tail->next=node;else queue->head=node;
    queue->tail=node;
}
win32_process_global_queue_node *win32_process_global_queue_pop(win32_process_global_queue *queue)
{
    win32_process_global_queue_node *node=queue->head;
    if(node){queue->head=node->next;if(!queue->head)queue->tail=NULL;node->next=NULL;}
    return node;
}
