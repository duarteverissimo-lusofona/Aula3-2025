
#ifndef SJC_H
#define SJC_H

void sjf_scheduler(uint32_t current_time_ms, queue_t *rq, pcb_t **cpu_task);

#endif //SJC_H
