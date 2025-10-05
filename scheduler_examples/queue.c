#include "queue.h"
#include <stdio.h>
#include <stdlib.h>

// Cria um novo PCB (Process Control Block)
pcb_t *new_pcb(pid_t pid, uint32_t sockfd, uint32_t time_ms) {
    pcb_t * new_task = malloc(sizeof(pcb_t));
    if (!new_task) return NULL;   // Falha de memória

    new_task->pid = pid;                 // Identificador do processo
    new_task->status = TASK_COMMAND;     // Estado inicial
    new_task->slice_start_ms = 0;        // Momento em que começou fatia de CPU (usado em RR/MLFQ)
    new_task->sockfd = sockfd;           // Socket de comunicação com a app
    new_task->time_ms = time_ms;         // Tempo total de execução pedido
    new_task->ellapsed_time_ms = 0;      // Tempo já executado
    new_task->priority = 0;              // Prioridade inicial (0=alta, 1=média, 2=baixa)
    return new_task;
}

// Adiciona um PCB no fim da fila (enqueue)
int enqueue_pcb(queue_t* q, pcb_t* task) {
    queue_elem_t* elem = malloc(sizeof(queue_elem_t));
    if (!elem) return 0;   // Falha de memória

    elem->pcb = task;      // Nó aponta para o PCB
    elem->next = NULL;

    if (q->tail) {
        q->tail->next = elem;   // Liga ao último
    } else {
        q->head = elem;         // Se fila estava vazia, este é o primeiro
    }
    q->tail = elem;             // Atualiza último
    return 1;
}

// Retira o primeiro PCB da fila (dequeue)
pcb_t* dequeue_pcb(queue_t* q) {
    if (!q || !q->head) return NULL;   // Fila vazia

    queue_elem_t* node = q->head;
    pcb_t* task = node->pcb;

    q->head = node->next;              // Avança para o próximo
    if (!q->head)                      // Se ficou vazia, zera tail
        q->tail = NULL;

    free(node);                        // Liberta o nó
    return task;                       // Retorna PCB
}

// Remove um nó específico da fila (não precisa ser o head)
queue_elem_t *remove_queue_elem(queue_t* q, queue_elem_t* elem) {
    queue_elem_t* it = q->head;
    queue_elem_t* prev = NULL;

    while (it != NULL) {
        if (it == elem) {   // Achou o nó a remover
            if (prev) {
                prev->next = it->next; // Salta o nó
            } else {
                q->head = it->next;    // Se era o primeiro
            }
            if (it == q->tail) {
                q->tail = prev;        // Se era o último
            }
            return it;                 // Retorna nó removido (ainda não liberta)
        }
        prev = it;
        it = it->next;
    }

    printf("Queue element not found in queue\n");
    return NULL;
}


/**
 * Procura na fila de prontos pelo processo com o menor tempo de execução total (time_ms),
 * remove-o da fila e retorna um ponteiro para o seu PCB.
 * Esta função é o coração do escalonador SJF.
 * Retorna NULL se a fila estiver vazia.
 */
pcb_t* remove_shortest_pcb(queue_t *rq) {
    // Passo 1: Lidar com o caso da fila estar vazia.
    if (rq == NULL || rq->head == NULL) {
        return NULL;
    }

    // Passo 2: Encontrar o elemento da fila com o PCB mais curto.
    queue_elem_t *current_elem = rq->head;
    queue_elem_t *shortest_elem = rq->head; // Assume que o primeiro é o mais curto para começar

    // Itera a partir do segundo elemento
    current_elem = current_elem->next;
    while (current_elem != NULL) {
        // Compara o tempo do burst do processo atual com o menor já encontrado
        if (current_elem->pcb->time_ms < shortest_elem->pcb->time_ms) {
            // Se encontrarmos um mais curto, atualizamos nosso ponteiro
            shortest_elem = current_elem;
        }
        current_elem = current_elem->next;
    }

    // Passo 3: Remover o elemento encontrado da fila.
    // Primeiro, guardamos o PCB que queremos retornar.
    pcb_t *task_to_run = shortest_elem->pcb;

    // Agora, usamos a sua função para remover o nó da lista.
    remove_queue_elem(rq, shortest_elem);

    // A sua função `remove_queue_elem` apenas desliga o nó da lista, mas não liberta a sua memória.
    // É nossa responsabilidade libertar o `queue_elem_t`.
    free(shortest_elem);

    // Passo 4: Retornar o PCB do processo mais curto.
    return task_to_run;
}