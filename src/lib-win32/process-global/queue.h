#ifndef WIN32_PROCESS_GLOBAL_QUEUE_H
#define WIN32_PROCESS_GLOBAL_QUEUE_H
/* Intrusive FIFO. Zero-initialize before use; the owner serializes all access
 * and manages node lifetime. */
typedef struct win32_process_global_queue_node {struct win32_process_global_queue_node *next;} win32_process_global_queue_node;
typedef struct win32_process_global_queue {win32_process_global_queue_node *head,*tail;} win32_process_global_queue;
void win32_process_global_queue_push(win32_process_global_queue *queue,win32_process_global_queue_node *node);
win32_process_global_queue_node *win32_process_global_queue_pop(win32_process_global_queue *queue);
#endif
