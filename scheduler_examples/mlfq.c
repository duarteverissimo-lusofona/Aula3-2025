#include "mlfq.h"
#include <stdio.h>
#include <stdlib.h>
#include "msg.h"
#include <unistd.h>

// Quantums por nível de prioridade (em ms)
static const uint32_t QUANTUM_MS[NUM_PRIORITY_LEVELS] = {
    250,   // Prioridade 0 (alta): quantum curto
    500,   // Prioridade 1 (média): quantum médio  
    1000   // Prioridade 2 (baixa): quantum longo
};

// Contador global para priority boost
static uint32_t boost_counter_ms = 0;

/**
 * @brief Encontra e remove o processo de MAIOR prioridade (menor valor) da fila
 */
static pcb_t* remove_highest_priority_pcb(queue_t *rq) {
    if (rq == NULL || rq->head == NULL) {
        return NULL;
    }

    queue_elem_t *current_elem = rq->head;
    queue_elem_t *highest_priority_elem = rq->head;

    // Procurar o processo com menor valor de priority (maior prioridade)
    current_elem = current_elem->next;
    while (current_elem != NULL) {
        if (current_elem->pcb->priority < highest_priority_elem->pcb->priority) {
            highest_priority_elem = current_elem;
        }
        current_elem = current_elem->next;
    }

    // Guardar o PCB
    pcb_t *task_to_run = highest_priority_elem->pcb;

    // Remover da fila
    queue_elem_t *removed = remove_queue_elem(rq, highest_priority_elem);
    if (removed) {
        free(removed);
    }

    return task_to_run;
}

/**
 * @brief Aplica priority boost: todos os processos voltam para prioridade máxima (0)
 */
static void priority_boost(queue_t *rq, pcb_t *cpu_task) {
    // Boost para processo em execução
    if (cpu_task != NULL) {
        cpu_task->priority = 0;
    }

    // Boost para todos os processos na fila de prontos
    queue_elem_t *elem = rq->head;
    while (elem != NULL) {
        elem->pcb->priority = 0;
        elem = elem->next;
    }
}

/**
 * @brief MLFQ Scheduler com priority boost e tratamento de I/O
 */
void mlfq_scheduler(uint32_t current_time_ms, queue_t *rq, pcb_t **cpu_task) {
    
    // ===== Incrementar contador de priority boost =====
    boost_counter_ms += TICKS_MS;

    // ===== PARTE 1: Gerir processo em execução =====
    if (*cpu_task) {
        // Atualizar tempo decorrido
        (*cpu_task)->ellapsed_time_ms += TICKS_MS;

        // Obter quantum baseado na prioridade atual
        int current_priority = (*cpu_task)->priority;
        if (current_priority < 0 || current_priority >= NUM_PRIORITY_LEVELS) {
            current_priority = 0;  // Segurança
        }
        
        uint32_t quantum = QUANTUM_MS[current_priority];
        uint32_t time_in_slice = current_time_ms - (*cpu_task)->slice_start_ms;

        // CASO 1: Processo TERMINOU
        if ((*cpu_task)->ellapsed_time_ms >= (*cpu_task)->time_ms) {
            // Liberar o processo
            msg_t msg = {
                .pid = (*cpu_task)->pid,
                .request = PROCESS_REQUEST_DONE,
                .time_ms = current_time_ms
            };
            if (write((*cpu_task)->sockfd, &msg, sizeof(msg_t)) != sizeof(msg_t)) {
                perror("write");
            }
            
            free(*cpu_task);
            *cpu_task = NULL;
        }
        // CASO 2: Usou TODO o quantum (CPU-bound)
        else if (time_in_slice >= quantum) {

            // DESCE DE PRIORIDADE (penaliza processos CPU-bound)
            int new_priority = current_priority + 1;
            if (new_priority >= NUM_PRIORITY_LEVELS) {
                new_priority = NUM_PRIORITY_LEVELS - 1;  // Fica na última fila
            }
            (*cpu_task)->priority = new_priority;

            // Reenfileirar com nova prioridade e libertar CPU
            enqueue_pcb(rq, *cpu_task);
            *cpu_task = NULL;
        }

    }

    // ===== PARTE 2: Priority Boost (anti-starvation) =====
    if (boost_counter_ms >= MLFQ_PRIORITY_BOOST_MS) {
        // Todos os processos passam para prioridade máxima (0)
        priority_boost(rq, *cpu_task);
        boost_counter_ms = 0;  // Reset do contador
        
        printf("[MLFQ] Priority boost aplicado aos %u ms\n", current_time_ms);
    }

    // ===== PARTE 3: Escolher próximo processo (Round Robin na maior prioridade) =====
    if (*cpu_task == NULL) {
        // Pegar processo com maior prioridade (menor valor)
        *cpu_task = remove_highest_priority_pcb(rq);
        
        if (*cpu_task != NULL) {
            // Marcar início do novo time slice
            (*cpu_task)->slice_start_ms = current_time_ms;
            
        }
    }
}