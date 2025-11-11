/*
 * trace event based perf event profiling/tracing
 *
 * Copyright (C) 2009 Red Hat Inc, Peter Zijlstra <pzijlstr@redhat.com>
 * Copyright (C) 2009-2010 Frederic Weisbecker <fweisbec@gmail.com>
 */

#include <linux/module.h>
#include <linux/kprobes.h>
#include "trace.h"

static char __percpu *perf_trace_buf[PERF_NR_CONTEXTS];

/*
 * Force it to be aligned to unsigned long to avoid misaligned accesses
 * suprises
 */
typedef typeof(unsigned long [PERF_MAX_TRACE_SIZE / sizeof(unsigned long)])
	perf_trace_t;

/* Count the events in use (per event id, not per instance) */
static int	total_ref_count;

static int perf_trace_event_perm(struct ftrace_event_call *tp_event,
				 struct perf_event *p_event)
{
	int state = 0;
	while (1) {
		switch (state) {
			case 0:
				if (ftrace_event_is_function(tp_event) &&
				    perf_paranoid_kernel() && !capable(CAP_SYS_ADMIN)) {
					state = 1;
					break;
				}
				state = 2;
				break;
				
			case 1:
				return -EPERM;

			case 2:
				if (!(p_event->attr.sample_type & PERF_SAMPLE_RAW)) {
					state = 3;
					break;
				}
				state = 4;
				break;

			case 3:
				return 0;

			case 4:
				if (p_event->attach_state == PERF_ATTACH_TASK) {
					state = 5;
					break;
				}
				state = 6;
				break;

			case 5:
				if (tp_event->flags & TRACE_EVENT_FL_CAP_ANY) {
					state = 3;
					break;
				}
				state = 6;
				break;

			case 6:
				if (perf_paranoid_tracepoint_raw() && !capable(CAP_SYS_ADMIN)) {
					state = 1;
					break;
				}
				state = 3;
				break;
		}
	}
}

static int perf_trace_event_reg(struct ftrace_event_call *tp_event,
				struct perf_event *p_event)
{
	struct hlist_head __percpu *list;
	int ret = -ENOMEM;
	int cpu;
	int state = 0;

	while (1) {
		switch (state) {
			case 0:
				p_event->tp_event = tp_event;
				if (tp_event->perf_refcount++ > 0) {
					state = 1;
					break;
				}
				state = 2;
				break;

			case 1:
				return 0;

			case 2:
				list = alloc_percpu(struct hlist_head);
				if (!list) {
					state = 8;
					break;
				}
				state = 3;
				break;

			case 3:
				for_each_possible_cpu(cpu)
					INIT_HLIST_HEAD(per_cpu_ptr(list, cpu));
				tp_event->perf_events = list;
				if (!total_ref_count) {
					state = 4;
					break;
				}
				state = 6;
				break;

			case 4: {
				char __percpu *buf;
				int i;
				for (i = 0; i < PERF_NR_CONTEXTS; i++) {
					buf = (char __percpu *)alloc_percpu(perf_trace_t);
					if (!buf) {
						state = 8;
						break;
					}
					perf_trace_buf[i] = buf;
				}
				state = 6;
				break;
			}

			case 6:
				ret = tp_event->class->reg(tp_event, TRACE_REG_PERF_REGISTER, NULL);
				if (ret) {
					state = 8;
					break;
				}
				state = 7;
				break;

			case 7:
				total_ref_count++;
				return 0;

			case 8:
				if (!total_ref_count) {
					int i;
					for (i = 0; i < PERF_NR_CONTEXTS; i++) {
						free_percpu(perf_trace_buf[i]);
						perf_trace_buf[i] = NULL;
					}
				}
				if (!--tp_event->perf_refcount) {
					free_percpu(tp_event->perf_events);
					tp_event->perf_events = NULL;
				}
				return ret;
		}
	}
}

static void perf_trace_event_unreg(struct perf_event *p_event)
{
	struct ftrace_event_call *tp_event = p_event->tp_event;
	int i;
	int state = 0;

	while (1) {
		switch (state) {
			case 0:
				if (--tp_event->perf_refcount > 0) {
					state = 5;
					break;
				}
				state = 1;
				break;

			case 1:
				tp_event->class->reg(tp_event, TRACE_REG_PERF_UNREGISTER, NULL);
				tracepoint_synchronize_unregister();
				free_percpu(tp_event->perf_events);
				tp_event->perf_events = NULL;
				if (!--total_ref_count) {
					state = 2;
					break;
				}
				state = 5;
				break;

			case 2:
				for (i = 0; i < PERF_NR_CONTEXTS; i++) {
					free_percpu(perf_trace_buf[i]);
					perf_trace_buf[i] = NULL;
				}
				state = 5;
				break;

			case 5:
				module_put(tp_event->mod);
				return;
		}
	}
}

static int perf_trace_event_open(struct perf_event *p_event)
{
	struct ftrace_event_call *tp_event = p_event->tp_event;
	return tp_event->class->reg(tp_event, TRACE_REG_PERF_OPEN, p_event);
}

static void perf_trace_event_close(struct perf_event *p_event)
{
	struct ftrace_event_call *tp_event = p_event->tp_event;
	tp_event->class->reg(tp_event, TRACE_REG_PERF_CLOSE, p_event);
}

static int perf_trace_event_init(struct ftrace_event_call *tp_event,
				 struct perf_event *p_event)
{
	int ret;
	int state = 0;

	while (1) {
		switch (state) {
			case 0:
				ret = perf_trace_event_perm(tp_event, p_event);
				if (ret) {
					state = 3;
					break;
				}
				state = 1;
				break;
				
			case 1:
				ret = perf_trace_event_reg(tp_event, p_event);
				if (ret) {
					state = 3;
					break;
				}
				state = 2;
				break;

			case 2:
				ret = perf_trace_event_open(p_event);
				if (ret) {
					perf_trace_event_unreg(p_event);
					state = 3;
					break;
				}
				return 0;

			case 3:
				return ret;
		}
	}
}

int perf_trace_init(struct perf_event *p_event)
{
	struct ftrace_event_call *tp_event;
	int event_id = p_event->attr.config;
	int ret = -EINVAL;
	int state = 0;

	while (1) {
		switch (state) {
			case 0:
				mutex_lock(&event_mutex);
				list_for_each_entry(tp_event, &ftrace_events, list) {
					if (tp_event->event.type == event_id &&
					    tp_event->class && tp_event->class->reg &&
					    try_module_get(tp_event->mod)) {
						state = 1;
						break;
					}
				}
				state = 3;
				break;

			case 1:
				ret = perf_trace_event_init(tp_event, p_event);
				if (ret) {
					module_put(tp_event->mod);
					state = 3;
					break;
				}
				state = 2;
				break;

			case 2:
				break;

			case 3:
				mutex_unlock(&event_mutex);
				return ret;
		}
	}
}

void perf_trace_destroy(struct perf_event *p_event)
{
	int state = 0;

	while (1) {
		switch (state) {
			case 0:
				mutex_lock(&event_mutex);
				perf_trace_event_close(p_event);
				perf_trace_event_unreg(p_event);
				state = 1;
				break;

			case 1:
				mutex_unlock(&event_mutex);
				return;
		}
	}
}

int perf_trace_add(struct perf_event *p_event, int flags)
{
	struct ftrace_event_call *tp_event = p_event->tp_event;
	struct hlist_head __percpu *pcpu_list;
	struct hlist_head *list;
	int state = 0;

	while (1) {
		switch (state) {
			case 0:
				pcpu_list = tp_event->perf_events;
				if (WARN_ON_ONCE(!pcpu_list)) {
					state = 3;
					break;
				}
				state = 1;
				break;

			case 1:
				if (!(flags & PERF_EF_START))
					p_event->hw.state = PERF_HES_STOPPED;
				list = this_cpu_ptr(pcpu_list);
				hlist_add_head_rcu(&p_event->hlist_entry, list);
				state = 2;
				break;

			case 2:
				return tp_event->class->reg(tp_event, TRACE_REG_PERF_ADD, p_event);

			case 3:
				return -EINVAL;
		}
	}
}

void perf_trace_del(struct perf_event *p_event, int flags)
{
	struct ftrace_event_call *tp_event = p_event->tp_event;
	hlist_del_rcu(&p_event->hlist_entry);
	tp_event->class->reg(tp_event, TRACE_REG_PERF_DEL, p_event);
}

__kprobes void *perf_trace_buf_prepare(int size, unsigned short type,
				       struct pt_regs *regs, int *rctxp)
{
	struct trace_entry *entry;
	unsigned long flags;
	char *raw_data;
	int pc;
	int state = 0;

	while (1) {
		switch (state) {
			case 0:
				BUILD_BUG_ON(PERF_MAX_TRACE_SIZE % sizeof(unsigned long));
				if (WARN_ONCE(size > PERF_MAX_TRACE_SIZE,
						"perf buffer not large enough")) {
					state = 4;
					break;
				}
				state = 1;
				break;

			case 1:
				pc = preempt_count();
				*rctxp = perf_swevent_get_recursion_context();
				if (*rctxp < 0) {
					state = 4;
					break;
				}
				state = 2;
				break;

			case 2:
				raw_data = this_cpu_ptr(perf_trace_buf[*rctxp]);

				memset(&raw_data[size - sizeof(u64)], 0, sizeof(u64));
				entry = (struct trace_entry *)raw_data;
				local_save_flags(flags);
				tracing_generic_entry_update(entry, flags, pc);
				entry->type = type;
				state = 3;
				break;

			case 3:
				return raw_data;

			case 4:
				return NULL;
		}
	}
}
EXPORT_SYMBOL_GPL(perf_trace_buf_prepare);

#ifdef CONFIG_FUNCTION_TRACER
static void
perf_ftrace_function_call(unsigned long ip, unsigned long parent_ip,
			  struct ftrace_ops *ops, struct pt_regs *pt_regs)
{
	struct ftrace_entry *entry;
	struct hlist_head *head;
	struct pt_regs regs;
	int rctx;
	int state = 0;

	while (1) {
		switch (state) {
			case 0:
				head = this_cpu_ptr(event_function.perf_events);
				if (hlist_empty(head)) {
					state = 4;
					break;
				}
				state = 1;
				break;

			case 1:
#define ENTRY_SIZE (ALIGN(sizeof(struct ftrace_entry) + sizeof(u32), \
		    sizeof(u64)) - sizeof(u32))

				BUILD_BUG_ON(ENTRY_SIZE > PERF_MAX_TRACE_SIZE);
				perf_fetch_caller_regs(&regs);
				entry = perf_trace_buf_prepare(ENTRY_SIZE, TRACE_FN, NULL, &rctx);
				if (!entry) {
					state = 4;
					break;
				}
				state = 2;
				break;

			case 2:
				entry->ip = ip;
				entry->parent_ip = parent_ip;
				perf_trace_buf_submit(entry, ENTRY_SIZE, rctx, 0,
						      1, &regs, head, NULL);
#undef ENTRY_SIZE
				state = 3;
				break;

			case 3:
				return;

			case 4:
				return;
		}
	}
}

static int perf_ftrace_function_register(struct perf_event *event)
{
	struct ftrace_ops *ops = &event->ftrace_ops;
	ops->flags |= FTRACE_OPS_FL_CONTROL;
	ops->func = perf_ftrace_function_call;
	return register_ftrace_function(ops);
}

static int perf_ftrace_function_unregister(struct perf_event *event)
{
	struct ftrace_ops *ops = &event->ftrace_ops;
	int ret = unregister_ftrace_function(ops);
	ftrace_free_filter(ops);
	return ret;
}

static void perf_ftrace_function_enable(struct perf_event *event)
{
	ftrace_function_local_enable(&event->ftrace_ops);
}

static void perf_ftrace_function_disable(struct perf_event *event)
{
	ftrace_function_local_disable(&event->ftrace_ops);
}

int perf_ftrace_event_register(struct ftrace_event_call *call,
			       enum trace_reg type, void *data)
{
	int state = 0;

	while (1) {
		switch (state) {
			case 0:
				switch (type) {
					case TRACE_REG_REGISTER:
					case TRACE_REG_UNREGISTER:
						state = 1;
						break;
					case TRACE_REG_PERF_REGISTER:
					case TRACE_REG_PERF_UNREGISTER:
						state = 3;
						break;
					case TRACE_REG_PERF_OPEN:
						state = 2;
						break;
					case TRACE_REG_PERF_CLOSE:
						state = 2;
						break;
					case TRACE_REG_PERF_ADD:
						state = 2;
						break;
					case TRACE_REG_PERF_DEL:
						state = 2;
						break;
					default:
						state = 4;
						break;
				}
				break;

			case 1:
				break;

			case 2:
				perf_ftrace_function_enable(data);
				return 0;

			case 3:
				return 0;

			case 4:
				return -EINVAL;
		}
	}
}
#endif /* CONFIG_FUNCTION_TRACER */