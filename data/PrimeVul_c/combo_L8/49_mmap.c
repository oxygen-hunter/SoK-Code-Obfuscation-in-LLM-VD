#include <Python.h>
#include <linux/personality.h>
#include <linux/mm.h>
#include <linux/random.h>
#include <linux/limits.h>
#include <linux/sched.h>
#include <asm/elf.h>

struct va_alignment __read_mostly va_align = {
	.flags = -1,
};

static unsigned int stack_maxrandom_size(void)
{
	unsigned int max = 0;
	if ((current->flags & PF_RANDOMIZE) &&
		!(current->personality & ADDR_NO_RANDOMIZE)) {
		max = ((-1U) & STACK_RND_MASK) << PAGE_SHIFT;
	}

	return max;
}

#define MIN_GAP (128*1024*1024UL + stack_maxrandom_size())
#define MAX_GAP (TASK_SIZE/6*5)

static int mmap_is_legacy(void)
{
	if (current->personality & ADDR_COMPAT_LAYOUT)
		return 1;

	if (rlimit(RLIMIT_STACK) == RLIM_INFINITY)
		return 1;

	return sysctl_legacy_va_layout;
}

static unsigned long mmap_rnd(void)
{
	unsigned long rnd = 0;

	if (current->flags & PF_RANDOMIZE) {
		if (mmap_is_ia32())
			rnd = get_random_int() % (1<<8);
		else
			rnd = get_random_int() % (1<<28);
	}
	return rnd << PAGE_SHIFT;
}

static unsigned long mmap_base(void)
{
	unsigned long gap = rlimit(RLIMIT_STACK);

	if (gap < MIN_GAP)
		gap = MIN_GAP;
	else if (gap > MAX_GAP)
		gap = MAX_GAP;

	return PAGE_ALIGN(TASK_SIZE - gap - mmap_rnd());
}

static unsigned long mmap_legacy_base(void)
{
	if (mmap_is_ia32())
		return TASK_UNMAPPED_BASE;
	else
		return TASK_UNMAPPED_BASE + mmap_rnd();
}

static PyObject* py_arch_pick_mmap_layout(PyObject* self, PyObject* args)
{
	struct mm_struct *mm;
	if (!PyArg_ParseTuple(args, "k", &mm))
		return NULL;

	mm->mmap_legacy_base = mmap_legacy_base();
	mm->mmap_base = mmap_base();

	if (mmap_is_legacy()) {
		mm->mmap_base = mm->mmap_legacy_base;
		mm->get_unmapped_area = (void*)arch_get_unmapped_area;
	} else {
		mm->get_unmapped_area = (void*)arch_get_unmapped_area_topdown;
	}

	Py_RETURN_NONE;
}

static PyMethodDef MmapMethods[] = {
	{"arch_pick_mmap_layout", py_arch_pick_mmap_layout, METH_VARARGS, "Selects VM layout function."},
	{NULL, NULL, 0, NULL}
};

static struct PyModuleDef mmapmodule = {
	PyModuleDef_HEAD_INIT,
	"mmapmodule",
	NULL,
	-1,
	MmapMethods
};

PyMODINIT_FUNC PyInit_mmapmodule(void)
{
	return PyModule_Create(&mmapmodule);
}