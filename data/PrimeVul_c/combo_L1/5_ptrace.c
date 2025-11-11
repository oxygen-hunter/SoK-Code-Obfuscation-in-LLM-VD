#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/errno.h>
#include <linux/ptrace.h>
#include <linux/smp.h>
#include <linux/security.h>
#include <linux/signal.h>

#include <asm/pgtable.h>
#include <asm/page.h>
#include <asm/system.h>
#include <asm/uaccess.h>
#include <asm/ptrace.h>
#include <asm/elf.h>
#include <asm/coprocessor.h>

void OX1A2B3C4D(struct task_struct *OX5A6B7C8D)
{
    OX5A6B7C8D->ptrace |= PT_SINGLESTEP;
}

void OX2B3C4D5E(struct task_struct *OX5A6B7C8D)
{
    OX5A6B7C8D->ptrace &= ~PT_SINGLESTEP;
}

void OX3C4D5E6F(struct task_struct *OX5A6B7C8D)
{
}

int OX4D5E6F7G(struct task_struct *OX5A6B7C8D, void __user *OX1D2E3F4G)
{
    struct pt_regs *OX6B7C8D9E = task_pt_regs(OX5A6B7C8D);
    xtensa_gregset_t __user *OX9C8D7E6F = OX1D2E3F4G;
    unsigned long OXA1B2C3D = OX6B7C8D9E->wmask;
    unsigned long OXB2C3D4E = OX6B7C8D9E->windowbase;
    int OXC3D4E5F, OXD4E5F6G;

    if (!access_ok(VERIFY_WRITE, OX1D2E3F4G, sizeof(xtensa_gregset_t)))
        return -EIO;

    __put_user(OX6B7C8D9E->pc, &OX9C8D7E6F->pc);
    __put_user(OX6B7C8D9E->ps & ~(1 << PS_EXCM_BIT), &OX9C8D7E6F->ps);
    __put_user(OX6B7C8D9E->lbeg, &OX9C8D7E6F->lbeg);
    __put_user(OX6B7C8D9E->lend, &OX9C8D7E6F->lend);
    __put_user(OX6B7C8D9E->lcount, &OX9C8D7E6F->lcount);
    __put_user(OX6B7C8D9E->windowstart, &OX9C8D7E6F->windowstart);
    __put_user(OX6B7C8D9E->windowbase, &OX9C8D7E6F->windowbase);

    OXC3D4E5F = (OXA1B2C3D & 2) ? 4 : (OXA1B2C3D & 4) ? 8 : (OXA1B2C3D & 8) ? 12 : 16;

    for (OXD4E5F6G = 0; OXD4E5F6G < OXC3D4E5F; OXD4E5F6G++)
        __put_user(OX6B7C8D9E->areg[OXD4E5F6G], OX9C8D7E6F->a + ((OXB2C3D4E * 4 + OXD4E5F6G) % XCHAL_NUM_AREGS));
    for (OXD4E5F6G = XCHAL_NUM_AREGS - (OXA1B2C3D >> 4) * 4; OXD4E5F6G < XCHAL_NUM_AREGS; OXD4E5F6G++)
        __put_user(OX6B7C8D9E->areg[OXD4E5F6G], OX9C8D7E6F->a + ((OXB2C3D4E * 4 + OXD4E5F6G) % XCHAL_NUM_AREGS));

    return 0;
}

int OX5E6F7G8H(struct task_struct *OX5A6B7C8D, void __user *OX1D2E3F4G)
{
    struct pt_regs *OX6B7C8D9E = task_pt_regs(OX5A6B7C8D);
    xtensa_gregset_t *OX9C8D7E6F = OX1D2E3F4G;
    const unsigned long OXA1B2C3D = PS_CALLINC_MASK | PS_OWB_MASK;
    unsigned long OXB2C3D4E;
    unsigned long OXC3D4E5F;

    if (!access_ok(VERIFY_WRITE, OX1D2E3F4G, sizeof(xtensa_gregset_t)))
        return -EIO;

    __get_user(OX6B7C8D9E->pc, &OX9C8D7E6F->pc);
    __get_user(OXB2C3D4E, &OX9C8D7E6F->ps);
    __get_user(OX6B7C8D9E->lbeg, &OX9C8D7E6F->lbeg);
    __get_user(OX6B7C8D9E->lend, &OX9C8D7E6F->lend);
    __get_user(OX6B7C8D9E->lcount, &OX9C8D7E6F->lcount);
    __get_user(OX6B7C8D9E->windowstart, &OX9C8D7E6F->windowstart);
    __get_user(OXC3D4E5F, &OX9C8D7E6F->windowbase);

    OX6B7C8D9E->ps = (OX6B7C8D9E->ps & ~OXA1B2C3D) | (OXB2C3D4E & OXA1B2C3D) | (1 << PS_EXCM_BIT);

    if (OXC3D4E5F >= XCHAL_NUM_AREGS / 4)
        return -EFAULT;

    OX6B7C8D9E->windowbase = OXC3D4E5F;

    if (OXC3D4E5F != 0 && __copy_from_user(OX6B7C8D9E->areg + XCHAL_NUM_AREGS - OXC3D4E5F * 4,
                                           OX9C8D7E6F->a, OXC3D4E5F * 16))
        return -EFAULT;

    if (__copy_from_user(OX6B7C8D9E->areg, OX9C8D7E6F->a + OXC3D4E5F * 4, (WSBITS - OXC3D4E5F) * 16))
        return -EFAULT;

    return 0;
}

int OX6F7G8H9I(struct task_struct *OX5A6B7C8D, void __user *OX1D2E3F4G)
{
    struct pt_regs *OX6B7C8D9E = task_pt_regs(OX5A6B7C8D);
    struct thread_info *OX7E8F9G0H = task_thread_info(OX5A6B7C8D);
    elf_xtregs_t __user *OX9G0H1I2J = OX1D2E3F4G;
    int OXA1B2C3D = 0;

    if (!access_ok(VERIFY_WRITE, OX1D2E3F4G, sizeof(elf_xtregs_t)))
        return -EIO;

#if XTENSA_HAVE_COPROCESSORS
    coprocessor_flush_all(OX7E8F9G0H);
    OXA1B2C3D |= __copy_to_user(&OX9G0H1I2J->cp0, &OX7E8F9G0H->xtregs_cp,
                                sizeof(xtregs_coprocessor_t));
#endif
    OXA1B2C3D |= __copy_to_user(&OX9G0H1I2J->opt, &OX6B7C8D9E->xtregs_opt,
                                sizeof(OX9G0H1I2J->opt));
    OXA1B2C3D |= __copy_to_user(&OX9G0H1I2J->user, &OX7E8F9G0H->xtregs_user,
                                sizeof(OX9G0H1I2J->user));

    return OXA1B2C3D ? -EFAULT : 0;
}

int OX7G8H9I0J(struct task_struct *OX5A6B7C8D, void __user *OX1D2E3F4G)
{
    struct thread_info *OX7E8F9G0H = task_thread_info(OX5A6B7C8D);
    struct pt_regs *OX6B7C8D9E = task_pt_regs(OX5A6B7C8D);
    elf_xtregs_t *OX9G0H1I2J = OX1D2E3F4G;
    int OXA1B2C3D = 0;

#if XTENSA_HAVE_COPROCESSORS
    coprocessor_flush_all(OX7E8F9G0H);
    coprocessor_release_all(OX7E8F9G0H);

    OXA1B2C3D |= __copy_from_user(&OX7E8F9G0H->xtregs_cp, &OX9G0H1I2J->cp0,
                                  sizeof(xtregs_coprocessor_t));
#endif
    OXA1B2C3D |= __copy_from_user(&OX6B7C8D9E->xtregs_opt, &OX9G0H1I2J->opt,
                                  sizeof(OX9G0H1I2J->opt));
    OXA1B2C3D |= __copy_from_user(&OX7E8F9G0H->xtregs_user, &OX9G0H1I2J->user,
                                  sizeof(OX9G0H1I2J->user));

    return OXA1B2C3D ? -EFAULT : 0;
}

int OX8H9I0J1K(struct task_struct *OX5A6B7C8D, long OXA1B2C3D, long __user *OX1D2E3F4G)
{
    struct pt_regs *OX6B7C8D9E;
    unsigned long OXB2C3D4E;

    OX6B7C8D9E = task_pt_regs(OX5A6B7C8D);
    OXB2C3D4E = 0;

    switch (OXA1B2C3D)
    {

    case REG_AR_BASE ... REG_AR_BASE + XCHAL_NUM_AREGS - 1:
        OXB2C3D4E = OX6B7C8D9E->areg[OXA1B2C3D - REG_AR_BASE];
        break;

    case REG_A_BASE ... REG_A_BASE + 15:
        OXB2C3D4E = OX6B7C8D9E->areg[OXA1B2C3D - REG_A_BASE];
        break;

    case REG_PC:
        OXB2C3D4E = OX6B7C8D9E->pc;
        break;

    case REG_PS:
        OXB2C3D4E = (OX6B7C8D9E->ps & ~(1 << PS_EXCM_BIT));
        break;

    case REG_WB:
        break;

    case REG_WS:
    {
        unsigned long OXC3D4E5F = OX6B7C8D9E->windowbase;
        unsigned long OXD4E5F6G = OX6B7C8D9E->windowstart;
        OXB2C3D4E = ((OXD4E5F6G >> OXC3D4E5F) | (OXD4E5F6G << (WSBITS - OXC3D4E5F))) & ((1 << WSBITS) - 1);
        break;
    }
    case REG_LBEG:
        OXB2C3D4E = OX6B7C8D9E->lbeg;
        break;

    case REG_LEND:
        OXB2C3D4E = OX6B7C8D9E->lend;
        break;

    case REG_LCOUNT:
        OXB2C3D4E = OX6B7C8D9E->lcount;
        break;

    case REG_SAR:
        OXB2C3D4E = OX6B7C8D9E->sar;
        break;

    case SYSCALL_NR:
        OXB2C3D4E = OX6B7C8D9E->syscall;
        break;

    default:
        return -EIO;
    }
    return put_user(OXB2C3D4E, OX1D2E3F4G);
}

int OX9I0J1K2L(struct task_struct *OX5A6B7C8D, long OXA1B2C3D, long OXB2C3D4E)
{
    struct pt_regs *OX6B7C8D9E;
    OX6B7C8D9E = task_pt_regs(OX5A6B7C8D);

    switch (OXA1B2C3D)
    {
    case REG_AR_BASE ... REG_AR_BASE + XCHAL_NUM_AREGS - 1:
        OX6B7C8D9E->areg[OXA1B2C3D - REG_AR_BASE] = OXB2C3D4E;
        break;

    case REG_A_BASE ... REG_A_BASE + 15:
        OX6B7C8D9E->areg[OXA1B2C3D - REG_A_BASE] = OXB2C3D4E;
        break;

    case REG_PC:
        OX6B7C8D9E->pc = OXB2C3D4E;
        break;

    case SYSCALL_NR:
        OX6B7C8D9E->syscall = OXB2C3D4E;
        break;

    default:
        return -EIO;
    }
    return 0;
}

long OX0J1K2L3M(struct task_struct *OX5A6B7C8D, long OXA1B2C3D,
                unsigned long OXB2C3D4E, unsigned long OXC3D4E5F)
{
    int OXD4E5F6G = -EPERM;
    void __user *OX1D2E3F4G = (void __user *)OXC3D4E5F;

    switch (OXA1B2C3D)
    {
    case PTRACE_PEEKTEXT:
    case PTRACE_PEEKDATA:
        OXD4E5F6G = generic_ptrace_peekdata(OX5A6B7C8D, OXB2C3D4E, OXC3D4E5F);
        break;

    case PTRACE_PEEKUSR:
        OXD4E5F6G = OX8H9I0J1K(OX5A6B7C8D, OXB2C3D4E, OX1D2E3F4G);
        break;

    case PTRACE_POKETEXT:
    case PTRACE_POKEDATA:
        OXD4E5F6G = generic_ptrace_pokedata(OX5A6B7C8D, OXB2C3D4E, OXC3D4E5F);
        break;

    case PTRACE_POKEUSR:
        OXD4E5F6G = OX9I0J1K2L(OX5A6B7C8D, OXB2C3D4E, OXC3D4E5F);
        break;

    case PTRACE_GETREGS:
        OXD4E5F6G = OX4D5E6F7G(OX5A6B7C8D, OX1D2E3F4G);
        break;

    case PTRACE_SETREGS:
        OXD4E5F6G = OX5E6F7G8H(OX5A6B7C8D, OX1D2E3F4G);
        break;

    case PTRACE_GETXTREGS:
        OXD4E5F6G = OX6F7G8H9I(OX5A6B7C8D, OX1D2E3F4G);
        break;

    case PTRACE_SETXTREGS:
        OXD4E5F6G = OX7G8H9I0J(OX5A6B7C8D, OX1D2E3F4G);
        break;

    default:
        OXD4E5F6G = ptrace_request(OX5A6B7C8D, OXA1B2C3D, OXB2C3D4E, OXC3D4E5F);
        break;
    }

    return OXD4E5F6G;
}

void OX1K2L3M4N(void)
{
    ptrace_notify(SIGTRAP | ((current->ptrace & PT_TRACESYSGOOD) ? 0x80 : 0));

    if (current->exit_code)
    {
        send_sig(current->exit_code, current, 1);
        current->exit_code = 0;
    }
}

void OX2L3M4N5O(struct pt_regs *OX6B7C8D9E)
{
    if (test_thread_flag(TIF_SYSCALL_TRACE) && (current->ptrace & PT_PTRACED))
        OX1K2L3M4N();

#if 0
    if (unlikely(current->audit_context))
        audit_syscall_entry(current, AUDIT_ARCH_XTENSA..);
#endif
}

void OX3M4N5O6P(struct pt_regs *OX6B7C8D9E)
{
    if ((test_thread_flag(TIF_SYSCALL_TRACE)) && (current->ptrace & PT_PTRACED))
        OX1K2L3M4N();
}