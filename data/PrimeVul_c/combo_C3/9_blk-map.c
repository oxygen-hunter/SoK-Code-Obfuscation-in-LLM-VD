#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/bio.h>
#include <linux/blkdev.h>
#include <linux/uio.h>

#include "blk.h"

#define STACK_SIZE 256
#define PROGRAM_SIZE 1024

enum {
	PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, RET,
	BLK_RQ_APPEND_BIO, BLK_RQ_MAP_USER_IOV, BLK_RQ_UNMAP_USER, BLK_RQ_MAP_KERN
};

typedef struct {
	int stack[STACK_SIZE];
	int sp;
	int pc;
	int program[PROGRAM_SIZE];
} VM;

void vm_init(VM *vm) {
	vm->sp = -1;
	vm->pc = 0;
}

void vm_run(VM *vm) {
	while (1) {
		switch (vm->program[vm->pc++]) {
		case PUSH:
			vm->stack[++vm->sp] = vm->program[vm->pc++];
			break;
		case POP:
			vm->sp--;
			break;
		case ADD:
			vm->stack[vm->sp-1] += vm->stack[vm->sp];
			vm->sp--;
			break;
		case SUB:
			vm->stack[vm->sp-1] -= vm->stack[vm->sp];
			vm->sp--;
			break;
		case JMP:
			vm->pc = vm->program[vm->pc];
			break;
		case JZ:
			if (vm->stack[vm->sp--] == 0)
				vm->pc = vm->program[vm->pc];
			else
				vm->pc++;
			break;
		case LOAD:
			vm->stack[++vm->sp] = vm->program[vm->pc++];
			break;
		case STORE:
			vm->program[vm->program[vm->pc++]] = vm->stack[vm->sp--];
			break;
		case CALL:
			switch (vm->program[vm->pc++]) {
			case BLK_RQ_APPEND_BIO:
				blk_rq_append_bio((struct request *)vm->stack[vm->sp-1], (struct bio *)vm->stack[vm->sp]);
				vm->sp -= 2;
				break;
			case BLK_RQ_MAP_USER_IOV:
				blk_rq_map_user_iov((struct request_queue *)vm->stack[vm->sp-4], (struct request *)vm->stack[vm->sp-3],
					(struct rq_map_data *)vm->stack[vm->sp-2], (const struct iov_iter *)vm->stack[vm->sp-1], (gfp_t)vm->stack[vm->sp]);
				vm->sp -= 5;
				break;
			case BLK_RQ_UNMAP_USER:
				blk_rq_unmap_user((struct bio *)vm->stack[vm->sp--]);
				break;
			case BLK_RQ_MAP_KERN:
				blk_rq_map_kern((struct request_queue *)vm->stack[vm->sp-4], (struct request *)vm->stack[vm->sp-3],
					(void *)vm->stack[vm->sp-2], (unsigned int)vm->stack[vm->sp-1], (gfp_t)vm->stack[vm->sp]);
				vm->sp -= 5;
				break;
			}
			break;
		case RET:
			return;
		}
	}
}

int blk_rq_append_bio(struct request *rq, struct bio *bio) {
	VM vm;
	vm_init(&vm);
	vm.program[0] = PUSH;
	vm.program[1] = (int)rq;
	vm.program[2] = PUSH;
	vm.program[3] = (int)bio;
	vm.program[4] = CALL;
	vm.program[5] = BLK_RQ_APPEND_BIO;
	vm.program[6] = RET;
	vm_run(&vm);
	return 0;
}
EXPORT_SYMBOL(blk_rq_append_bio);

static int __blk_rq_unmap_user(struct bio *bio) {
	VM vm;
	vm_init(&vm);
	vm.program[0] = PUSH;
	vm.program[1] = (int)bio;
	vm.program[2] = CALL;
	vm.program[3] = BLK_RQ_UNMAP_USER;
	vm.program[4] = RET;
	vm_run(&vm);
	return 0;
}

static int __blk_rq_map_user_iov(struct request *rq, struct rq_map_data *map_data, struct iov_iter *iter, gfp_t gfp_mask, bool copy) {
	VM vm;
	vm_init(&vm);
	vm.program[0] = PUSH;
	vm.program[1] = (int)rq;
	vm.program[2] = PUSH;
	vm.program[3] = (int)map_data;
	vm.program[4] = PUSH;
	vm.program[5] = (int)iter;
	vm.program[6] = PUSH;
	vm.program[7] = (int)gfp_mask;
	vm.program[8] = PUSH;
	vm.program[9] = (int)copy;
	vm.program[10] = CALL;
	vm.program[11] = BLK_RQ_MAP_USER_IOV;
	vm.program[12] = RET;
	vm_run(&vm);
	return 0;
}

int blk_rq_map_user_iov(struct request_queue *q, struct request *rq, struct rq_map_data *map_data, const struct iov_iter *iter, gfp_t gfp_mask) {
	VM vm;
	vm_init(&vm);
	vm.program[0] = PUSH;
	vm.program[1] = (int)q;
	vm.program[2] = PUSH;
	vm.program[3] = (int)rq;
	vm.program[4] = PUSH;
	vm.program[5] = (int)map_data;
	vm.program[6] = PUSH;
	vm.program[7] = (int)iter;
	vm.program[8] = PUSH;
	vm.program[9] = (int)gfp_mask;
	vm.program[10] = CALL;
	vm.program[11] = BLK_RQ_MAP_USER_IOV;
	vm.program[12] = RET;
	vm_run(&vm);
	return 0;
}
EXPORT_SYMBOL(blk_rq_map_user_iov);

int blk_rq_map_user(struct request_queue *q, struct request *rq, struct rq_map_data *map_data, void __user *ubuf, unsigned long len, gfp_t gfp_mask) {
	VM vm;
	vm_init(&vm);
	vm.program[0] = PUSH;
	vm.program[1] = (int)q;
	vm.program[2] = PUSH;
	vm.program[3] = (int)rq;
	vm.program[4] = PUSH;
	vm.program[5] = (int)map_data;
	vm.program[6] = PUSH;
	vm.program[7] = (int)ubuf;
	vm.program[8] = PUSH;
	vm.program[9] = (int)len;
	vm.program[10] = PUSH;
	vm.program[11] = (int)gfp_mask;
	vm.program[12] = CALL;
	vm.program[13] = BLK_RQ_MAP_USER_IOV;
	vm.program[14] = RET;
	vm_run(&vm);
	return 0;
}
EXPORT_SYMBOL(blk_rq_map_user);

int blk_rq_unmap_user(struct bio *bio) {
	VM vm;
	vm_init(&vm);
	vm.program[0] = PUSH;
	vm.program[1] = (int)bio;
	vm.program[2] = CALL;
	vm.program[3] = BLK_RQ_UNMAP_USER;
	vm.program[4] = RET;
	vm_run(&vm);
	return 0;
}
EXPORT_SYMBOL(blk_rq_unmap_user);

int blk_rq_map_kern(struct request_queue *q, struct request *rq, void *kbuf, unsigned int len, gfp_t gfp_mask) {
	VM vm;
	vm_init(&vm);
	vm.program[0] = PUSH;
	vm.program[1] = (int)q;
	vm.program[2] = PUSH;
	vm.program[3] = (int)rq;
	vm.program[4] = PUSH;
	vm.program[5] = (int)kbuf;
	vm.program[6] = PUSH;
	vm.program[7] = (int)len;
	vm.program[8] = PUSH;
	vm.program[9] = (int)gfp_mask;
	vm.program[10] = CALL;
	vm.program[11] = BLK_RQ_MAP_KERN;
	vm.program[12] = RET;
	vm_run(&vm);
	return 0;
}
EXPORT_SYMBOL(blk_rq_map_kern);