#include "rr.h"
#include <stdio.h>
#include <stdlib.h>
#include "msg.h"
#include <unistd.h>

#define QUANTUM_MS 500  // Time slice de 500ms (0.5 segundos)

/**
 * @brief Round Robin scheduling algorithm with time slice preemption.
 *
 * Se o CPU está ocupado:
 * - Verifica se o processo terminou (ellapsed >= time_ms) -> envia DONE e liberta
 * - Verifica se o quantum expirou -> preempta e reenfileira
 * 
 * Se o CPU está livre:
 * - Pega o próximo processo da fila (FIFO dentro do RR)
 */
void rr_scheduler(uint32_t current_time_ms, queue_t *rq, pcb_t **cpu_task) {
    if (*cpu_task) {
        // CPU está ocupado - atualizar tempo decorrido
        (*cpu_task)->ellapsed_time_ms += TICKS_MS;
        (*cpu_task)->slice_start_ms += TICKS_MS;

        // Verificar se o processo TERMINOU
        if ((*cpu_task)->ellapsed_time_ms >= (*cpu_task)->time_ms) {
            // Processo completou - enviar DONE
            msg_t msg = {
                .pid = (*cpu_task)->pid,
                .request = PROCESS_REQUEST_DONE,
                .time_ms = current_time_ms
            };
            if (write((*cpu_task)->sockfd, &msg, sizeof(msg_t)) != sizeof(msg_t)) {
                perror("write");
            }
            
            // Libertar o PCB
            free((*cpu_task));
            (*cpu_task) = NULL;
        } 
        // Verificar se o QUANTUM EXPIROU (preempção)
        else if ((*cpu_task)->slice_start_ms >= QUANTUM_MS) {
            // Quantum expirou - fazer preempção

            // NÃO enviar DONE aqui: o processo não terminou, apenas foi preemptado.
            // Se o protocolo suportar, pode-se enviar um PREEMPT/YIELD; caso contrário, nada.

            (*cpu_task)->slice_start_ms = 0;  // Resetar contador do time slice (assignment, não comparação)
            // Reenfileirar o processo no fim da fila (preempção)
            enqueue_pcb(rq, *cpu_task);
            (*cpu_task) = NULL;  // Libertar CPU
        }
    }

    // Se CPU está livre, pegar próximo processo da fila
    if (*cpu_task == NULL) {
        *cpu_task = dequeue_pcb(rq);
        
        // Se pegamos um novo processo, iniciar contador do time slice em 0
        if (*cpu_task != NULL) {
            (*cpu_task)->slice_start_ms = 0;
        }
    }
}