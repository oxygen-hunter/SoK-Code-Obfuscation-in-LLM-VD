#include <linux/mm.h>
#include <linux/export.h>
#include <linux/sysctl.h>
#include <linux/nsproxy.h>
#include <net/sock.h>
#ifdef CONFIG_INET
#include <net/ip.h>
#endif
#ifdef CONFIG_NET
#include <linux/if_ether.h>
#endif

typedef enum { PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, RET, HALT } OpCode;

typedef struct {
	int stack[256];
	int sp;
	int pc;
	int program[1024];
	int memory[256];
} VM;

void vm_init(VM *vm) {
	vm->sp = -1;
	vm->pc = 0;
	for (int i = 0; i < 256; i++) {
		vm->stack[i] = 0;
		vm->memory[i] = 0;
	}
}

void vm_push(VM *vm, int value) {
	vm->stack[++vm->sp] = value;
}

int vm_pop(VM *vm) {
	return vm->stack[vm->sp--];
}

void vm_execute(VM *vm) {
	int running = 1;
	while (running) {
		switch (vm->program[vm->pc++]) {
		case PUSH:
			vm_push(vm, vm->program[vm->pc++]);
			break;
		case POP:
			vm_pop(vm);
			break;
		case ADD: {
			int b = vm_pop(vm);
			int a = vm_pop(vm);
			vm_push(vm, a + b);
			break;
		}
		case SUB: {
			int b = vm_pop(vm);
			int a = vm_pop(vm);
			vm_push(vm, a - b);
			break;
		}
		case JMP:
			vm->pc = vm->program[vm->pc];
			break;
		case JZ: {
			int addr = vm->program[vm->pc++];
			if (vm_pop(vm) == 0) vm->pc = addr;
			break;
		}
		case LOAD: {
			int addr = vm->program[vm->pc++];
			vm_push(vm, vm->memory[addr]);
			break;
		}
		case STORE: {
			int addr = vm->program[vm->pc++];
			vm->memory[addr] = vm_pop(vm);
			break;
		}
		case CALL: {
			int addr = vm->program[vm->pc++];
			vm_push(vm, vm->pc);
			vm->pc = addr;
			break;
		}
		case RET:
			vm->pc = vm_pop(vm);
			break;
		case HALT:
			running = 0;
			break;
		}
	}
}

void setup_vm(VM *vm) {
	vm_init(vm);
	int program[] = {
		PUSH, 0, CALL, 100, HALT, // main
		PUSH, 0, LOAD, PUSH, 0, LOAD, ADD, STORE, 0, RET, // net_ctl_permissions
		PUSH, 1, LOAD, PUSH, 2, LOAD, ADD, STORE, 1, RET // net_ctl_header_lookup
	};
	for (int i = 0; i < sizeof(program) / sizeof(int); i++) {
		vm->program[i] = program[i];
	}
}

static struct ctl_table_set *net_ctl_header_lookup(struct ctl_table_root *root, struct nsproxy *namespaces) {
	VM vm;
	setup_vm(&vm);
	vm.memory[0] = (int)&namespaces->net_ns->sysctls;
	vm_execute(&vm);
	return (struct ctl_table_set *)vm.memory[1];
}

static int net_ctl_permissions(struct ctl_table_header *head, struct ctl_table *table) {
	VM vm;
	setup_vm(&vm);
	vm.memory[0] = (int)&current->nsproxy->net_ns->sysctls == (int)&head->set;
	vm_execute(&vm);
	return vm.memory[1];
}

static struct ctl_table_root net_sysctl_root = {
	.lookup = net_ctl_header_lookup,
	.permissions = net_ctl_permissions,
};

static int __net_init sysctl_net_init(struct net *net) {
	setup_sysctl_set(&net->sysctls, &net_sysctl_root, net_ctl_permissions);
	return 0;
}

static void __net_exit sysctl_net_exit(struct net *net) {
	retire_sysctl_set(&net->sysctls);
}

static struct pernet_operations sysctl_pernet_ops = {
	.init = sysctl_net_init,
	.exit = sysctl_net_exit,
};

static struct ctl_table_header *net_header;
__init int net_sysctl_init(void) {
	static struct ctl_table empty[1];
	int ret = -ENOMEM;
	net_header = register_sysctl("net", empty);
	if (!net_header) goto out;
	ret = register_pernet_subsys(&sysctl_pernet_ops);
	if (ret) goto out;
	register_sysctl_root(&net_sysctl_root);
out:
	return ret;
}

struct ctl_table_header *register_net_sysctl(struct net *net, const char *path, struct ctl_table *table) {
	return __register_sysctl_table(&net->sysctls, path, table);
}
EXPORT_SYMBOL_GPL(register_net_sysctl);

void unregister_net_sysctl_table(struct ctl_table_header *header) {
	unregister_sysctl_table(header);
}
EXPORT_SYMBOL_GPL(unregister_net_sysctl_table);