#include "common/TableLookUp.h"
#include "decoders/RawDecoderException.h" 
#include <cassert>                        

namespace rawspeed {

struct TableConfig { bool dither; int ntables; };
TableConfig tableConfig;
std::vector<ushort16> tables;

const int LOOKUP_SIZE = 65536 * 2;

TableLookUp::TableLookUp(int _ntables, bool _dither) {
  tableConfig = {_dither, _ntables};
  if (tableConfig.ntables < 1) {
    ThrowRDE("Cannot construct 0 tables");
  }
  tables.resize(tableConfig.ntables * LOOKUP_SIZE, ushort16(0));
}

void TableLookUp::setTable(int ntable, const std::vector<ushort16>& table) {
  assert(!table.empty());

  const int nfilled = table.size();
  if (nfilled >= 65536)
    ThrowRDE("Table lookup with %i entries is unsupported", nfilled);

  if (ntable > tableConfig.ntables) {
    ThrowRDE("Table lookup with number greater than number of tables.");
  }
  ushort16* t = &tables[ntable * LOOKUP_SIZE];
  if (!tableConfig.dither) {
    for (int i = 0; i < 65536; i++) {
      t[i] = (i < nfilled) ? table[i] : table[nfilled - 1];
    }
    return;
  }
  for (int i = 0; i < nfilled; i++) {
    int upper = i < (nfilled - 1) ? table[i + 1] : table[i];
    int center = table[i];
    int lower = i > 0 ? table[i - 1] : center;
    int delta = upper - lower;
    t[i * 2] = center - ((upper - lower + 2) / 4);
    t[i * 2 + 1] = delta;
  }

  for (int i = nfilled; i < 65536; i++) {
    t[i * 2] = table[nfilled - 1];
    t[i * 2 + 1] = 0;
  }
  t[0] = t[1];
  t[LOOKUP_SIZE - 1] = t[LOOKUP_SIZE - 2];
}

ushort16* TableLookUp::getTable(int n) {
  if (n > tableConfig.ntables) {
    ThrowRDE("Table lookup with number greater than number of tables.");
  }
  return &tables[n * LOOKUP_SIZE];
}

} // namespace rawspeed