#include <Python.h>
#include "PlayListPLS.h"

extern "C" {
  // C function declarations
  bool LoadPLSFile(const char* filename);
  void SavePLSFile(const char* filename);
}

// Python wrapper function for LoadPLSFile
static PyObject* py_LoadPLSFile(PyObject* self, PyObject* args) {
  const char* filename;
  if (!PyArg_ParseTuple(args, "s", &filename))
    return NULL;
  bool result = LoadPLSFile(filename);
  return PyBool_FromLong(result);
}

// Python wrapper function for SavePLSFile
static PyObject* py_SavePLSFile(PyObject* self, PyObject* args) {
  const char* filename;
  if (!PyArg_ParseTuple(args, "s", &filename))
    return NULL;
  SavePLSFile(filename);
  Py_RETURN_NONE;
}

static PyMethodDef PlaylistMethods[] = {
  {"load_pls", py_LoadPLSFile, METH_VARARGS, "Load a PLS playlist file."},
  {"save_pls", py_SavePLSFile, METH_VARARGS, "Save a PLS playlist file."},
  {NULL, NULL, 0, NULL}
};

static struct PyModuleDef playlistmodule = {
  PyModuleDef_HEAD_INIT,
  "playlist",
  NULL,
  -1,
  PlaylistMethods
};

PyMODINIT_FUNC PyInit_playlist(void) {
  return PyModule_Create(&playlistmodule);
}

bool LoadPLSFile(const char* filename) {
  CPlayListPLS playlist;
  return playlist.Load(filename);
}

void SavePLSFile(const char* filename) {
  CPlayListPLS playlist;
  playlist.Save(filename);
}

int main(int argc, char* argv[]) {
  PyImport_AppendInittab("playlist", PyInit_playlist);
  Py_Initialize();
  PyRun_SimpleString("import playlist\n"
                     "playlist.load_pls('example.pls')\n"
                     "playlist.save_pls('example_saved.pls')\n");
  Py_Finalize();
  return 0;
}