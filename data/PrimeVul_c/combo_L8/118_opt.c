#include <Python.h>
#include <stdbool.h>
#include <limits.h>
#include <errno.h>
#include <assert.h>

// Example C function to be called from Python
static PyObject* c_wait_for_option(PyObject* self, PyObject* args) {
    // Assuming h is passed as an integer for demonstration purposes
    int h;
    if (!PyArg_ParseTuple(args, "i", &h)) {
        return NULL;
    }

    while (nbd_internal_is_state_connecting(get_next_state((struct nbd_handle*)h))) {
        if (nbd_unlocked_poll((struct nbd_handle*)h, -1) == -1)
            return PyLong_FromLong(-1);
    }

    return PyLong_FromLong(0);
}

// Method definitions for Python module
static PyMethodDef NBDMethods[] = {
    {"wait_for_option", c_wait_for_option, METH_VARARGS, "Wait for an option in NBD"},
    {NULL, NULL, 0, NULL}
};

// Module definition
static struct PyModuleDef nbdmodule = {
    PyModuleDef_HEAD_INIT, "nbd", NULL, -1, NBDMethods
};

// Module initialization function
PyMODINIT_FUNC PyInit_nbd(void) {
    return PyModule_Create(&nbdmodule);
}

void
nbd_internal_free_option (struct nbd_handle *h)
{
  if (h->opt_current == NBD_OPT_LIST)
    FREE_CALLBACK (h->opt_cb.fn.list);
  else if (h->opt_current == NBD_OPT_LIST_META_CONTEXT)
    FREE_CALLBACK (h->opt_cb.fn.context);
  FREE_CALLBACK (h->opt_cb.completion);
}

int
nbd_unlocked_set_opt_mode (struct nbd_handle *h, bool value)
{
  h->opt_mode = value;
  return 0;
}

int
nbd_unlocked_get_opt_mode (struct nbd_handle *h)
{
  return h->opt_mode;
}

int
nbd_unlocked_opt_go (struct nbd_handle *h)
{
  int err;
  nbd_completion_callback c = { .callback = go_complete, .user_data = &err };
  int r = nbd_unlocked_aio_opt_go (h, &c);

  if (r == -1)
    return r;

  r = PyObject_CallMethod(nbdmodule.m_base.m_copy, "wait_for_option", "i", h);
  if (r == 0 && err) {
    assert (nbd_internal_is_state_negotiating (get_next_state (h)));
    set_error (err, "server replied with error to opt_go request");
    return -1;
  }
  if (r == 0)
    assert (nbd_internal_is_state_ready (get_next_state (h)));
  return r;
}

int
nbd_unlocked_opt_info (struct nbd_handle *h)
{
  int err;
  nbd_completion_callback c = { .callback = go_complete, .user_data = &err };
  int r = nbd_unlocked_aio_opt_info (h, &c);

  if (r == -1)
    return r;

  r = PyObject_CallMethod(nbdmodule.m_base.m_copy, "wait_for_option", "i", h);
  if (r == -1 || (r == 0 && err)) {
    set_error (err, "server replied with error to opt_info request");
    return -1;
  }
  return r;
}

int
nbd_unlocked_opt_abort (struct nbd_handle *h)
{
  int r = nbd_unlocked_aio_opt_abort (h);

  if (r == -1)
    return r;

  return PyObject_CallMethod(nbdmodule.m_base.m_copy, "wait_for_option", "i", h);
}

int
nbd_unlocked_opt_list (struct nbd_handle *h, nbd_list_callback *list)
{
  struct list_helper s = { .list = *list };
  nbd_list_callback l = { .callback = list_visitor, .user_data = &s };
  nbd_completion_callback c = { .callback = list_complete, .user_data = &s };

  if (nbd_unlocked_aio_opt_list (h, &l, &c) == -1)
    return -1;

  SET_CALLBACK_TO_NULL (*list);
  int r = PyObject_CallMethod(nbdmodule.m_base.m_copy, "wait_for_option", "i", h);
  if (r == -1 || s.err) {
    set_error (s.err, "server replied with error to list request");
    return -1;
  }
  return s.count;
}

int
nbd_unlocked_opt_list_meta_context (struct nbd_handle *h, nbd_context_callback *context)
{
  struct context_helper s = { .context = *context };
  nbd_context_callback l = { .callback = context_visitor, .user_data = &s };
  nbd_completion_callback c = { .callback = context_complete, .user_data = &s };

  if (nbd_unlocked_aio_opt_list_meta_context (h, &l, &c) == -1)
    return -1;

  SET_CALLBACK_TO_NULL (*context);
  int r = PyObject_CallMethod(nbdmodule.m_base.m_copy, "wait_for_option", "i", h);
  if (r == -1 || s.err) {
    set_error (s.err, "server replied with error to list meta context request");
    return -1;
  }
  return s.count;
}

int
nbd_unlocked_aio_opt_go (struct nbd_handle *h, nbd_completion_callback *complete)
{
  h->opt_current = NBD_OPT_GO;
  h->opt_cb.completion = *complete;
  SET_CALLBACK_TO_NULL (*complete);

  if (nbd_internal_run (h, cmd_issue) == -1)
    debug (h, "option queued, ignoring state machine failure");
  return 0;
}

int
nbd_unlocked_aio_opt_info (struct nbd_handle *h, nbd_completion_callback *complete)
{
  if ((h->gflags & LIBNBD_HANDSHAKE_FLAG_FIXED_NEWSTYLE) == 0) {
    set_error (ENOTSUP, "server is not using fixed newstyle protocol");
    return -1;
  }

  h->opt_current = NBD_OPT_INFO;
  h->opt_cb.completion = *complete;
  SET_CALLBACK_TO_NULL (*complete);

  if (nbd_internal_run (h, cmd_issue) == -1)
    debug (h, "option queued, ignoring state machine failure");
  return 0;
}

int
nbd_unlocked_aio_opt_abort (struct nbd_handle *h)
{
  h->opt_current = NBD_OPT_ABORT;

  if (nbd_internal_run (h, cmd_issue) == -1)
    debug (h, "option queued, ignoring state machine failure");
  return 0;
}

int
nbd_unlocked_aio_opt_list (struct nbd_handle *h, nbd_list_callback *list, nbd_completion_callback *complete)
{
  if ((h->gflags & LIBNBD_HANDSHAKE_FLAG_FIXED_NEWSTYLE) == 0) {
    set_error (ENOTSUP, "server is not using fixed newstyle protocol");
    return -1;
  }

  assert (CALLBACK_IS_NULL (h->opt_cb.fn.list));
  h->opt_cb.fn.list = *list;
  SET_CALLBACK_TO_NULL (*list);
  h->opt_cb.completion = *complete;
  SET_CALLBACK_TO_NULL (*complete);
  h->opt_current = NBD_OPT_LIST;
  if (nbd_internal_run (h, cmd_issue) == -1)
    debug (h, "option queued, ignoring state machine failure");
  return 0;
}

int
nbd_unlocked_aio_opt_list_meta_context (struct nbd_handle *h, nbd_context_callback *context, nbd_completion_callback *complete)
{
  if ((h->gflags & LIBNBD_HANDSHAKE_FLAG_FIXED_NEWSTYLE) == 0) {
    set_error (ENOTSUP, "server is not using fixed newstyle protocol");
    return -1;
  }
  if (!h->structured_replies) {
    set_error (ENOTSUP, "server lacks structured replies");
    return -1;
  }

  assert (CALLBACK_IS_NULL (h->opt_cb.fn.context));
  h->opt_cb.fn.context = *context;
  SET_CALLBACK_TO_NULL (*context);
  h->opt_cb.completion = *complete;
  SET_CALLBACK_TO_NULL (*complete);
  h->opt_current = NBD_OPT_LIST_META_CONTEXT;
  if (nbd_internal_run (h, cmd_issue) == -1)
    debug (h, "option queued, ignoring state machine failure");
  return 0;
}