#include "common/TableLookUp.h"
#include "decoders/RawDecoderException.h"
#include <cassert>

namespace rawspeed {

const int getTableSize() {
  return 65536 * 2;
}

TableLookUp::TableLookUp(int _ntables, bool _dither)
    : ntables(_ntables), dither(_dither) {
  if (ntables < 1) {
    ThrowRDE("Cannot construct 0 tables");
  }
  tables.resize(getDynamicTableSize(), ushort16(0));
}

int getDynamicTableSize() {
  return ntables * getTableSize();
}

void TableLookUp::setTable(int ntable, const std::vector<ushort16>& table) {
  assert(!table.empty());

  const int nfilled = getFilledSize(table);
  if (nfilled >= 65536)
    ThrowRDE("Table lookup with %i entries is unsupported", nfilled);

  if (ntable > ntables) {
    ThrowRDE("Table lookup with number greater than number of tables.");
  }
  ushort16* t = getTablePointer(ntable);
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
  t[getTableSize() - 1] = t[getTableSize() - 2];
}

ushort16* TableLookUp::getTable(int n) {
  if (n > ntables) {
    ThrowRDE("Table lookup with number greater than number of tables.");
  }
  return getTablePointer(n);
}

int getFilledSize(const std::vector<ushort16>& table) {
  return table.size();
}

ushort16* getTablePointer(int n) {
  return &tables[n * getTableSize()];
}

} // namespace rawspeed