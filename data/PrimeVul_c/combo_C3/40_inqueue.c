#include <net/sctp/sctp.h>
#include <net/sctp/sm.h>
#include <linux/interrupt.h>
#include <linux/slab.h>

enum {
    VM_PUSH, VM_POP, VM_ADD, VM_SUB, VM_JMP, VM_JZ, VM_LOAD, VM_STORE,
    VM_INIT_QUEUE, VM_FREE_QUEUE, VM_PUSH_PACKET, VM_PEEK_CHUNK, VM_POP_CHUNK,
    VM_SET_TH_HANDLER, VM_END
};

struct VM {
    int stack[256];
    int sp;
    int pc;
    int program[1024];
    struct sctp_inq *queue;
    struct sctp_chunk *chunk;
    struct sctp_chunkhdr *chunkhdr;
    work_func_t callback;
};

void vm_init(struct VM *vm) {
    vm->sp = -1;
    vm->pc = 0;
}

void vm_run(struct VM *vm) {
    for (;;) {
        switch (vm->program[vm->pc++]) {
            case VM_PUSH:
                vm->stack[++vm->sp] = vm->program[vm->pc++];
                break;
            case VM_POP:
                --vm->sp;
                break;
            case VM_ADD:
                vm->stack[vm->sp - 1] += vm->stack[vm->sp];
                --vm->sp;
                break;
            case VM_SUB:
                vm->stack[vm->sp - 1] -= vm->stack[vm->sp];
                --vm->sp;
                break;
            case VM_JMP:
                vm->pc = vm->program[vm->pc];
                break;
            case VM_JZ:
                if (vm->stack[vm->sp--] == 0)
                    vm->pc = vm->program[vm->pc];
                else
                    vm->pc++;
                break;
            case VM_LOAD:
                vm->stack[++vm->sp] = vm->queue->immediate.func;
                break;
            case VM_STORE:
                vm->queue->immediate.func = vm->stack[vm->sp--];
                break;
            case VM_INIT_QUEUE:
                INIT_LIST_HEAD(&vm->queue->in_chunk_list);
                vm->queue->in_progress = NULL;
                INIT_WORK(&vm->queue->immediate, NULL);
                break;
            case VM_FREE_QUEUE:
                {
                    struct sctp_chunk *chunk, *tmp;
                    list_for_each_entry_safe(chunk, tmp, &vm->queue->in_chunk_list, list) {
                        list_del_init(&chunk->list);
                        sctp_chunk_free(chunk);
                    }
                    if (vm->queue->in_progress) {
                        sctp_chunk_free(vm->queue->in_progress);
                        vm->queue->in_progress = NULL;
                    }
                }
                break;
            case VM_PUSH_PACKET:
                if (vm->chunk->rcvr->dead) {
                    sctp_chunk_free(vm->chunk);
                    break;
                }
                list_add_tail(&vm->chunk->list, &vm->queue->in_chunk_list);
                if (vm->chunk->asoc)
                    vm->chunk->asoc->stats.ipackets++;
                vm->queue->immediate.func(&vm->queue->immediate);
                break;
            case VM_PEEK_CHUNK:
                vm->chunk = vm->queue->in_progress;
                if (!vm->chunk || vm->chunk->singleton || vm->chunk->end_of_packet || vm->chunk->pdiscard)
                    vm->chunkhdr = NULL;
                else
                    vm->chunkhdr = (sctp_chunkhdr_t *)vm->chunk->chunk_end;
                break;
            case VM_POP_CHUNK:
                {
                    struct sctp_chunk *chunk;
                    sctp_chunkhdr_t *ch = NULL;
                    if ((chunk = vm->queue->in_progress)) {
                        if (chunk->singleton || chunk->end_of_packet || chunk->pdiscard) {
                            sctp_chunk_free(chunk);
                            chunk = vm->queue->in_progress = NULL;
                        } else {
                            ch = (sctp_chunkhdr_t *) chunk->chunk_end;
                            skb_pull(chunk->skb, chunk->chunk_end - chunk->skb->data);
                            if (skb_headlen(chunk->skb) < sizeof(sctp_chunkhdr_t)) {
                                sctp_chunk_free(chunk);
                                chunk = vm->queue->in_progress = NULL;
                            }
                        }
                    }
                    if (!chunk) {
                        struct list_head *entry;
                        if (list_empty(&vm->queue->in_chunk_list))
                            break;
                        entry = vm->queue->in_chunk_list.next;
                        chunk = vm->queue->in_progress = list_entry(entry, struct sctp_chunk, list);
                        list_del_init(entry);
                        chunk->singleton = 1;
                        ch = (sctp_chunkhdr_t *) chunk->skb->data;
                        chunk->data_accepted = 0;
                    }
                    chunk->chunk_hdr = ch;
                    chunk->chunk_end = ((__u8 *)ch) + WORD_ROUND(ntohs(ch->length));
                    if (unlikely(skb_is_nonlinear(chunk->skb))) {
                        if (chunk->chunk_end > skb_tail_pointer(chunk->skb))
                            chunk->chunk_end = skb_tail_pointer(chunk->skb);
                    }
                    skb_pull(chunk->skb, sizeof(sctp_chunkhdr_t));
                    chunk->subh.v = NULL;
                    if (chunk->chunk_end < skb_tail_pointer(chunk->skb)) {
                        chunk->singleton = 0;
                    } else if (chunk->chunk_end > skb_tail_pointer(chunk->skb)) {
                        sctp_chunk_free(chunk);
                        chunk = vm->queue->in_progress = NULL;
                        break;
                    } else {
                        chunk->end_of_packet = 1;
                    }
                    pr_debug("+++sctp_inq_pop+++ chunk:%p[%s], length:%d, skb->len:%d\n",
                             chunk, sctp_cname(SCTP_ST_CHUNK(chunk->chunk_hdr->type)),
                             ntohs(chunk->chunk_hdr->length), chunk->skb->len);
                    vm->chunk = chunk;
                }
                break;
            case VM_SET_TH_HANDLER:
                INIT_WORK(&vm->queue->immediate, vm->callback);
                break;
            case VM_END:
                return;
        }
    }
}

void sctp_inq_init(struct sctp_inq *queue)
{
    struct VM vm;
    vm_init(&vm);
    vm.queue = queue;
    vm.program[0] = VM_INIT_QUEUE;
    vm.program[1] = VM_END;
    vm_run(&vm);
}

void sctp_inq_free(struct sctp_inq *queue)
{
    struct VM vm;
    vm_init(&vm);
    vm.queue = queue;
    vm.program[0] = VM_FREE_QUEUE;
    vm.program[1] = VM_END;
    vm_run(&vm);
}

void sctp_inq_push(struct sctp_inq *q, struct sctp_chunk *chunk)
{
    struct VM vm;
    vm_init(&vm);
    vm.queue = q;
    vm.chunk = chunk;
    vm.program[0] = VM_PUSH_PACKET;
    vm.program[1] = VM_END;
    vm_run(&vm);
}

struct sctp_chunkhdr *sctp_inq_peek(struct sctp_inq *queue)
{
    struct VM vm;
    vm_init(&vm);
    vm.queue = queue;
    vm.program[0] = VM_PEEK_CHUNK;
    vm.program[1] = VM_END;
    vm_run(&vm);
    return vm.chunkhdr;
}

struct sctp_chunk *sctp_inq_pop(struct sctp_inq *queue)
{
    struct VM vm;
    vm_init(&vm);
    vm.queue = queue;
    vm.program[0] = VM_POP_CHUNK;
    vm.program[1] = VM_END;
    vm_run(&vm);
    return vm.chunk;
}

void sctp_inq_set_th_handler(struct sctp_inq *q, work_func_t callback)
{
    struct VM vm;
    vm_init(&vm);
    vm.queue = q;
    vm.callback = callback;
    vm.program[0] = VM_SET_TH_HANDLER;
    vm.program[1] = VM_END;
    vm_run(&vm);
}