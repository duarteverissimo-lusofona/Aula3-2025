#include "fifo.h"

#include <stdio.h>
#include <stdlib.h>

#include "msg.h"
#include <unistd.h>

/**
 * @brief First-In-First-Out (FIFO) scheduling algorithm.
 *
 * This function implements the FIFO scheduling algorithm. If the CPU is not idle it
 * checks if the application is ready and frees the CPU.
 * If the CPU is idle, it selects the next task to run based on the order they were added
 * to the ready queue. The task that has been in the queue the longest is selected to run next.
 *
 * @param current_time_ms The current time in milliseconds.
 * @param rq Pointer to the ready queue containing tasks that are ready to run.
 * @param cpu_task Double pointer to the currently running task. This will be updated
 *                 to point to the next task to run.
 */
void fifo_scheduler(uint32_t current_time_ms, queue_t *rq, pcb_t **cpu_task) {
    // Verifica se já existe uma tarefa a correr no CPU
    if (*cpu_task) {
        // Atualiza o tempo já gasto pela tarefa atual
        (*cpu_task)->ellapsed_time_ms += TICKS_MS;

        // Verifica se a tarefa já completou o seu tempo total de execução
        if ((*cpu_task)->ellapsed_time_ms >= (*cpu_task)->time_ms) {
            // Criar mensagem a informar que o processo terminou
            msg_t msg = {
                .pid = (*cpu_task)->pid,
                .request = PROCESS_REQUEST_DONE,
                .time_ms = current_time_ms
            };

            // Enviar a mensagem para a aplicação associada
            if (write((*cpu_task)->sockfd, &msg, sizeof(msg_t)) != sizeof(msg_t)) {
                perror("write");
            }

            // Libertar a memória do processo terminado
            free((*cpu_task));
            (*cpu_task) = NULL;  // Marcar CPU como livre
        }
    }

    // Se o CPU está livre, atribuir a próxima tarefa da fila
    if (*cpu_task == NULL) {
        *cpu_task = dequeue_pcb(rq);   // Retira o primeiro da fila (FIFO)
    }
}
