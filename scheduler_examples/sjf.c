#include "fifo.h"

#include <stdio.h>
#include <stdlib.h>

#include "msg.h"
#include <unistd.h>


/**
 * Escalonador SJF (Shortest Job First) Não-Preemptivo.
 * Esta função é chamada a cada tick do sistema para tomar decisões de escalonamento.
 */

void sjf_scheduler(uint32_t current_time_ms, queue_t *rq, pcb_t **cpu_task) {
    // PARTE 1: Gerir o processo que já está a correr
    if (*cpu_task) {
        // Atualiza o tempo já gasto pela tarefa atual
        (*cpu_task)->ellapsed_time_ms += TICKS_MS; // Assumindo que TICKS_MS é uma constante definida

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

    // PARTE 2: Se o CPU está livre, escolher o próximo processo
    if (*cpu_task == NULL) {

        *cpu_task = remove_shortest_pcb(rq);
    }
}