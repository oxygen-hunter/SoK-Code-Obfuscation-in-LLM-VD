#include "common/TableLookUp.h"
#include "decoders/RawDecoderException.h"
#include <cassert>

extern "C" {
#include <Python.h>
}

namespace rawspeed {

using ushort16 = unsigned short;

const int TABLE_SIZE = 65536 * 2;

class TableLookUp {
public:
  TableLookUp(int _ntables, bool _dither);
  void setTable(int ntable, const std::vector<ushort16>& table);
  ushort16* getTable(int n);
private:
  std::vector<ushort16> tables;
  int ntables;
  bool dither;
};

TableLookUp::TableLookUp(int _ntables, bool _dither)
    : ntables(_ntables), dither(_dither) {
  if (ntables < 1) {
    PyErr_SetString(PyExc_RuntimeError, "Cannot construct 0 tables");
    throw PyExc_RuntimeError;
  }
  tables.resize(ntables * TABLE_SIZE, ushort16(0));
}

void TableLookUp::setTable(int ntable, const std::vector<ushort16>& table) {
  assert(!table.empty());

  const int nfilled = table.size();
  if (nfilled >= 65536) {
    PyErr_SetString(PyExc_RuntimeError, "Table lookup with too many entries");
    throw PyExc_RuntimeError;
  }

  if (ntable > ntables) {
    PyErr_SetString(PyExc_RuntimeError, "Table lookup number too large");
    throw PyExc_RuntimeError;
  }
  ushort16* t = &tables[ntable * TABLE_SIZE];
  if (!dither) {
    for (int i = 0; i < 65536; i++) {
      t[i] = (i < nfilled) ? table[i] : table[nfilled - 1];
    }
    return;
  }
  for (int i = 0; i < nfilled; i++) {
    int center = table[i];
    int lower = i > 0 ? table[i - 1] : center;
    int upper = i < (nfilled - 1) ? table[i + 1] : center;
    int delta = upper - lower;
    t[i * 2] = center - ((upper - lower + 2) / 4);
    t[i * 2 + 1] = delta;
  }

  for (int i = nfilled; i < 65536; i++) {
    t[i * 2] = table[nfilled - 1];
    t[i * 2 + 1] = 0;
  }
  t[0] = t[1];
  t[TABLE_SIZE - 1] = t[TABLE_SIZE - 2];
}

ushort16* TableLookUp::getTable(int n) {
  if (n > ntables) {
    PyErr_SetString(PyExc_RuntimeError, "Table lookup number too large");
    throw PyExc_RuntimeError;
  }
  return &tables[n * TABLE_SIZE];
}

} // namespace rawspeed