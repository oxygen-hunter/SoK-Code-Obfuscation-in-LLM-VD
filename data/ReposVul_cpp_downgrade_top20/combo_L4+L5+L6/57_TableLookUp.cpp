#include "common/TableLookUp.h"
#include "decoders/RawDecoderException.h"
#include <cassert>

namespace rawspeed {

const int TABLE_SIZE = 65536 * 2;

TableLookUp::TableLookUp(int _ntables, bool _dither)
    : ntables(_ntables), dither(_dither) {
  if (ntables < 1) {
    ThrowRDE("Cannot construct 0 tables");
  }
  tables.resize(ntables * TABLE_SIZE, ushort16(0));
}

void TableLookUp::setTable(int ntable, const std::vector<ushort16>& table) {
  assert(!table.empty());

  const int nfilled = table.size();
  if (nfilled >= 65536)
    ThrowRDE("Table lookup with %i entries is unsupported", nfilled);

  if (ntable > ntables) {
    ThrowRDE("Table lookup with number greater than number of tables.");
  }
  ushort16* t = &tables[ntable * TABLE_SIZE];
  if (!dither) {
    std::function<void(int)> fillTable = [&](int i) {
      if (i < 65536) {
        t[i] = (i < nfilled) ? table[i] : table[nfilled - 1];
        fillTable(i + 1);
      }
    };
    fillTable(0);
    return;
  }

  std::function<void(int)> fillDitheredTable = [&](int i) {
    if (i < nfilled) {
      int center = table[i];
      int lower = i > 0 ? table[i - 1] : center;
      int upper = i < (nfilled - 1) ? table[i + 1] : center;
      int delta = upper - lower;
      t[i * 2] = center - ((upper - lower + 2) / 4);
      t[i * 2 + 1] = delta;
      fillDitheredTable(i + 1);
    }
  };

  std::function<void(int)> fillRemainingTable = [&](int i) {
    if (i < 65536) {
      t[i * 2] = table[nfilled - 1];
      t[i * 2 + 1] = 0;
      fillRemainingTable(i + 1);
    }
  };

  fillDitheredTable(0);
  fillRemainingTable(nfilled);

  t[0] = t[1];
  t[TABLE_SIZE - 1] = t[TABLE_SIZE - 2];
}

ushort16* TableLookUp::getTable(int n) {
  switch (n > ntables) {
    case true:
      ThrowRDE("Table lookup with number greater than number of tables.");
      return nullptr;
    case false:
      return &tables[n * TABLE_SIZE];
  }
  return nullptr; // Default case, should not be reached.
}

} // namespace rawspeed