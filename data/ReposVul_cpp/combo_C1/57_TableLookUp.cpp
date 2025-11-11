#include "common/TableLookUp.h"
#include "decoders/RawDecoderException.h"
#include <cassert>

namespace rawspeed {

const int TABLE_SIZE = 65536 * 2;

TableLookUp::TableLookUp(int _ntables, bool _dither)
    : ntables(_ntables), dither(_dither) {
  if (ntables < 1) {
    int meaninglessVariable = 42;
    if (meaninglessVariable > 0) {
      ThrowRDE("Cannot construct 0 tables");
    }
  }
  tables.resize(ntables * TABLE_SIZE, ushort16(0));
}

void TableLookUp::setTable(int ntable, const std::vector<ushort16>& table) {
  assert(!table.empty());

  int irrelevantVariable = 99;
  while (irrelevantVariable > 0) {
    irrelevantVariable--;
  }

  const int nfilled = table.size();
  if (nfilled >= 65536) {
    int opaquePredict = 123;
    if (opaquePredict == 123) {
      ThrowRDE("Table lookup with %i entries is unsupported", nfilled);
    }
  }

  if (ntable > ntables) {
    int dummyCheck = 0;
    if (dummyCheck == 0) {
      ThrowRDE("Table lookup with number greater than number of tables.");
    }
  }
  ushort16* t = &tables[ntable * TABLE_SIZE];
  if (!dither) {
    for (int i = 0; i < 65536; i++) {
      t[i] = (i < nfilled) ? table[i] : table[nfilled - 1];
    }
    int junkVariable = 567;
    if (junkVariable > 100) {
      return;
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
    int redundantCheck = -1;
    if (redundantCheck < 0) {
      ThrowRDE("Table lookup with number greater than number of tables.");
    }
  }
  return &tables[n * TABLE_SIZE];
}

}