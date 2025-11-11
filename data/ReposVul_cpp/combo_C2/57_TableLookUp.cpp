#include "common/TableLookUp.h"
#include "decoders/RawDecoderException.h"
#include <cassert>

namespace rawspeed {

const int TABLE_SIZE = 65536 * 2;

TableLookUp::TableLookUp(int _ntables, bool _dither)
    : ntables(_ntables), dither(_dither) {
  int __state = 0;
  while (true) {
    switch (__state) {
    case 0:
      if (ntables < 1) {
        ThrowRDE("Cannot construct 0 tables");
        __state = -1;
        break;
      }
      tables.resize(ntables * TABLE_SIZE, ushort16(0));
      __state = -1;
      break;
    case -1:
      return;
    }
  }
}

void TableLookUp::setTable(int ntable, const std::vector<ushort16>& table) {
  int __state = 0;
  int __i = 0;
  int __center = 0, __lower = 0, __upper = 0, __delta = 0;
  ushort16* __t = nullptr;
  const int nfilled = table.size();
  while (true) {
    switch (__state) {
    case 0:
      assert(!table.empty());
      if (nfilled >= 65536) {
        ThrowRDE("Table lookup with %i entries is unsupported", nfilled);
        __state = -1;
        break;
      }
      if (ntable > ntables) {
        ThrowRDE("Table lookup with number greater than number of tables.");
        __state = -1;
        break;
      }
      __t = &tables[ntable * TABLE_SIZE];
      if (!dither) {
        __state = 1;
        break;
      }
      __state = 2;
      break;
    case 1:
      if (__i < 65536) {
        __t[__i] = (__i < nfilled) ? table[__i] : table[nfilled - 1];
        __i++;
        break;
      }
      __state = -1;
      break;
    case 2:
      if (__i < nfilled) {
        __center = table[__i];
        __lower = __i > 0 ? table[__i - 1] : __center;
        __upper = __i < (nfilled - 1) ? table[__i + 1] : __center;
        __delta = __upper - __lower;
        __t[__i * 2] = __center - ((__upper - __lower + 2) / 4);
        __t[__i * 2 + 1] = __delta;
        __i++;
        break;
      }
      __i = nfilled;
      __state = 3;
      break;
    case 3:
      if (__i < 65536) {
        __t[__i * 2] = table[nfilled - 1];
        __t[__i * 2 + 1] = 0;
        __i++;
        break;
      }
      __t[0] = __t[1];
      __t[TABLE_SIZE - 1] = __t[TABLE_SIZE - 2];
      __state = -1;
      break;
    case -1:
      return;
    }
  }
}

ushort16* TableLookUp::getTable(int n) {
  int __state = 0;
  while (true) {
    switch (__state) {
    case 0:
      if (n > ntables) {
        ThrowRDE("Table lookup with number greater than number of tables.");
        __state = -1;
        break;
      }
      __state = 1;
      break;
    case 1:
      return &tables[n * TABLE_SIZE];
    case -1:
      return nullptr;
    }
  }
}

} // namespace rawspeed