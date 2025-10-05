#ifndef MLFQ_H
#define MLFQ_H

#include "queue.h"

#define NUM_PRIORITY_LEVELS 3           // 0 (alta), 1 (média), 2 (baixa)
#define MLFQ_PRIORITY_BOOST_MS 5000     // A cada 5 segundos, boost de prioridade

/**
 * @brief Multi-Level Feedback Queue scheduler
 * 
 * Regras:
 * - Processos começam com prioridade 0 (máxima)
 * - Se consumir todo o quantum: desce prioridade
 * - Se terminar/bloquear antes do quantum: mantém prioridade
 * - A cada MLFQ_PRIORITY_BOOST_MS: todos voltam para prioridade 0
 * - Sempre executa processos de maior prioridade primeiro
 * 
 * @param current_time_ms Tempo atual da simulação
 * @param rq Fila única com processos de diferentes prioridades
 * @param cpu_task Ponteiro para o processo em execução no CPU
 */
void mlfq_scheduler(uint32_t current_time_ms, queue_t *rq, pcb_t **cpu_task);

#endif //MLFQ_H