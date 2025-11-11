#include "nfsd.h"
#include <linux/nfsacl.h>
#include <linux/gfp.h>
#include "cache.h"
#include "xdr3.h"
#include "vfs.h"

#define NFSDDBG_FACILITY		NFSDDBG_PROC
#define RETURN_STATUS(st)	{ resp->status = (st); return (st); }

enum {
    PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, HALT
};

typedef struct {
    int pc;
    int stack[256];
    int sp;
    int memory[256];
} VM;

void vm_init(VM *vm) {
    vm->pc = 0;
    vm->sp = -1;
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

void vm_execute(VM *vm, int *program) {
    while (1) {
        switch (program[vm->pc++]) {
            case PUSH:
                vm_push(vm, program[vm->pc++]);
                break;
            case POP:
                vm_pop(vm);
                break;
            case ADD: {
                int a = vm_pop(vm);
                int b = vm_pop(vm);
                vm_push(vm, a + b);
                break;
            }
            case SUB: {
                int a = vm_pop(vm);
                int b = vm_pop(vm);
                vm_push(vm, b - a);
                break;
            }
            case JMP:
                vm->pc = program[vm->pc];
                break;
            case JZ:
                if (vm_pop(vm) == 0) {
                    vm->pc = program[vm->pc];
                } else {
                    vm->pc++;
                }
                break;
            case LOAD:
                vm_push(vm, vm->memory[program[vm->pc++]]);
                break;
            case STORE:
                vm->memory[program[vm->pc++]] = vm_pop(vm);
                break;
            case HALT:
                return;
        }
    }
}

int getacl_program[] = {
    PUSH, 1, // Placeholder for the logic
    HALT
};

int setacl_program[] = {
    PUSH, 2, // Placeholder for the logic
    HALT
};

int getattr_program[] = {
    PUSH, 3, // Placeholder for the logic
    HALT
};

int access_program[] = {
    PUSH, 4, // Placeholder for the logic
    HALT
};

static __be32
nfsacld_proc_null(struct svc_rqst *rqstp, void *argp, void *resp)
{
    return nfs_ok;
}

static __be32 nfsacld_proc_getacl(struct svc_rqst * rqstp,
		struct nfsd3_getaclargs *argp, struct nfsd3_getaclres *resp)
{
    VM vm;
    vm_init(&vm);
    vm_execute(&vm, getacl_program);
    return 0;
}

static __be32 nfsacld_proc_setacl(struct svc_rqst * rqstp,
		struct nfsd3_setaclargs *argp, struct nfsd_attrstat *resp)
{
    VM vm;
    vm_init(&vm);
    vm_execute(&vm, setacl_program);
    return 0;
}

static __be32 nfsacld_proc_getattr(struct svc_rqst * rqstp,
		struct nfsd_fhandle *argp, struct nfsd_attrstat *resp)
{
    VM vm;
    vm_init(&vm);
    vm_execute(&vm, getattr_program);
    return 0;
}

static __be32 nfsacld_proc_access(struct svc_rqst *rqstp, struct nfsd3_accessargs *argp,
		struct nfsd3_accessres *resp)
{
    VM vm;
    vm_init(&vm);
    vm_execute(&vm, access_program);
    return 0;
}

#define nfsaclsvc_decode_voidargs	NULL
#define nfsaclsvc_release_void		NULL
#define nfsd3_fhandleargs	nfsd_fhandle
#define nfsd3_attrstatres	nfsd_attrstat
#define nfsd3_voidres		nfsd3_voidargs
struct nfsd3_voidargs { int dummy; };

#define PROC(name, argt, rest, relt, cache, respsize)	\
 { (svc_procfunc) nfsacld_proc_##name,		\
   (kxdrproc_t) nfsaclsvc_decode_##argt##args,	\
   (kxdrproc_t) nfsaclsvc_encode_##rest##res,	\
   (kxdrproc_t) nfsaclsvc_release_##relt,		\
   sizeof(struct nfsd3_##argt##args),		\
   sizeof(struct nfsd3_##rest##res),		\
   0,						\
   cache,					\
   respsize,					\
 }

#define ST 1		/* status*/
#define AT 21		/* attributes */
#define pAT (1+AT)	/* post attributes - conditional */
#define ACL (1+NFS_ACL_MAX_ENTRIES*3)  /* Access Control List */

static struct svc_procedure		nfsd_acl_procedures2[] = {
  PROC(null,	void,		void,		void,	  RC_NOCACHE, ST),
  PROC(getacl,	getacl,		getacl,		getacl,	  RC_NOCACHE, ST+1+2*(1+ACL)),
  PROC(setacl,	setacl,		attrstat,	attrstat, RC_NOCACHE, ST+AT),
  PROC(getattr, fhandle,	attrstat,	attrstat, RC_NOCACHE, ST+AT),
  PROC(access,	access,		access,		access,   RC_NOCACHE, ST+AT+1),
};

struct svc_version	nfsd_acl_version2 = {
		.vs_vers	= 2,
		.vs_nproc	= 5,
		.vs_proc	= nfsd_acl_procedures2,
		.vs_dispatch	= nfsd_dispatch,
		.vs_xdrsize	= NFS3_SVC_XDRSIZE,
		.vs_hidden	= 0,
};