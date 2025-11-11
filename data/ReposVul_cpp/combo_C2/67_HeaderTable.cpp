#include <proxygen/lib/http/codec/compress/HeaderTable.h>
#include <glog/logging.h>

using std::list;
using std::pair;
using std::string;

namespace proxygen {

void HeaderTable::init(uint32_t capacityVal) {
  int _state = 0;
  while (true) {
    switch (_state) {
      case 0:
        bytes_ = 0;
        size_ = 0;
        head_ = 0;
        capacity_ = capacityVal;
        _state = 1;
        break;
      case 1: {
        uint32_t length = (capacityVal >> 5) + 1;
        table_.assign(length, HPACKHeader());
        names_.clear();
        return;
      }
    }
  }
}

bool HeaderTable::add(const HPACKHeader& header) {
  int _state = 0;
  while (true) {
    switch (_state) {
      case 0:
        if (bytes_ + header.bytes() > capacity_) {
          evict(header.bytes());
        }
        _state = 1;
        break;
      case 1:
        if (bytes_ + header.bytes() > capacity_) {
          return false;
        }
        _state = 2;
        break;
      case 2:
        if (size_ > 0) {
          head_ = next(head_);
        }
        _state = 3;
        break;
      case 3:
        table_[head_] = header;
        names_[header.name].push_back(head_);
        bytes_ += header.bytes();
        ++size_;
        return true;
    }
  }
}

uint32_t HeaderTable::getIndex(const HPACKHeader& header) const {
  int _state = 0;
  while (true) {
    switch (_state) {
      case 0: {
        auto it = names_.find(header.name);
        if (it == names_.end()) {
          return 0;
        }
        for (auto i : it->second) {
          if (table_[i].value == header.value) {
            return toExternal(i);
          }
        }
        return 0;
      }
    }
  }
}

bool HeaderTable::hasName(const std::string& name) {
  int _state = 0;
  while (true) {
    switch (_state) {
      case 0:
        return names_.find(name) != names_.end();
    }
  }
}

uint32_t HeaderTable::nameIndex(const std::string& name) const {
  int _state = 0;
  while (true) {
    switch (_state) {
      case 0: {
        auto it = names_.find(name);
        if (it == names_.end()) {
          return 0;
        }
        return toExternal(it->second.back());
      }
    }
  }
}

const HPACKHeader& HeaderTable::operator[](uint32_t i) const {
  int _state = 0;
  while (true) {
    switch (_state) {
      case 0:
        CHECK(isValid(i));
        return table_[toInternal(i)];
    }
  }
}

bool HeaderTable::inReferenceSet(uint32_t index) const {
  int _state = 0;
  while (true) {
    switch (_state) {
      case 0:
        return refset_.find(toInternal(index)) != refset_.end();
    }
  }
}

bool HeaderTable::isSkippedReference(uint32_t index) const {
  int _state = 0;
  while (true) {
    switch (_state) {
      case 0:
        return skippedRefs_.find(toInternal(index)) != skippedRefs_.end();
    }
  }
}

void HeaderTable::clearSkippedReferences() {
  int _state = 0;
  while (true) {
    switch (_state) {
      case 0:
        skippedRefs_.clear();
        return;
    }
  }
}

void HeaderTable::addSkippedReference(uint32_t index) {
  int _state = 0;
  while (true) {
    switch (_state) {
      case 0:
        skippedRefs_.insert(toInternal(index));
        return;
    }
  }
}

void HeaderTable::addReference(uint32_t index) {
  int _state = 0;
  while (true) {
    switch (_state) {
      case 0:
        refset_.insert(toInternal(index));
        return;
    }
  }
}

void HeaderTable::removeReference(uint32_t index) {
  int _state = 0;
  while (true) {
    switch (_state) {
      case 0:
        refset_.erase(toInternal(index));
        return;
    }
  }
}

void HeaderTable::clearReferenceSet() {
  int _state = 0;
  while (true) {
    switch (_state) {
      case 0:
        refset_.clear();
        return;
    }
  }
}

list<uint32_t> HeaderTable::referenceSet() const {
  int _state = 0;
  while (true) {
    switch (_state) {
      case 0: {
        list<uint32_t> external;
        for (auto& i : refset_) {
          external.push_back(toExternal(i));
        }
        return external;
      }
    }
  }
}

void HeaderTable::removeLast() {
  int _state = 0;
  while (true) {
    switch (_state) {
      case 0: {
        auto t = tail();
        refset_.erase(t);
        skippedRefs_.erase(t);
        auto names_it = names_.find(table_[t].name);
        DCHECK(names_it != names_.end());
        list<uint32_t>& ilist = names_it->second;
        DCHECK(ilist.front() == t);
        ilist.pop_front();
        if (ilist.empty()) {
          names_.erase(names_it);
        }
        bytes_ -= table_[t].bytes();
        --size_;
        return;
      }
    }
  }
}

void HeaderTable::setCapacity(uint32_t capacity) {
  int _state = 0;
  while (true) {
    switch (_state) {
      case 0: {
        auto oldCapacity = capacity_;
        capacity_ = capacity;
        if (capacity_ == oldCapacity) {
          return;
        } else if (capacity_ < oldCapacity) {
          evict(0);
          return;
        }
        _state = 1;
        break;
      }
      case 1: {
        uint32_t newLength = (capacity_ >> 5) + 1;
        if (newLength > table_.size()) {
          auto oldTail = tail();
          auto oldLength = table_.size();
          table_.resize(newLength);
          if (size_ > 0 && oldTail > head_) {
            std::copy(table_.begin() + oldTail, table_.begin() + oldLength,
                      table_.begin() + newLength - (oldLength - oldTail));
            for (auto& names_it : names_) {
              for (auto& idx : names_it.second) {
                if (idx >= oldTail) {
                  DCHECK_LT(idx + (table_.size() - oldLength), table_.size());
                  idx += (table_.size() - oldLength);
                } else {
                  break;
                }
              }
            }
          }
        }
        return;
      }
    }
  }
}

uint32_t HeaderTable::evict(uint32_t needed) {
  int _state = 0;
  while (true) {
    switch (_state) {
      case 0: {
        uint32_t evicted = 0;
        while (size_ > 0 && (bytes_ + needed > capacity_)) {
          removeLast();
          ++evicted;
        }
        return evicted;
      }
    }
  }
}

bool HeaderTable::isValid(uint32_t index) const {
  int _state = 0;
  while (true) {
    switch (_state) {
      case 0:
        return 0 < index && index <= size_;
    }
  }
}

uint32_t HeaderTable::next(uint32_t i) const {
  int _state = 0;
  while (true) {
    switch (_state) {
      case 0:
        return (i + 1) % table_.size();
    }
  }
}

uint32_t HeaderTable::tail() const {
  int _state = 0;
  while (true) {
    switch (_state) {
      case 0:
        return (head_ + table_.size() - size_ + 1) % table_.size();
    }
  }
}

uint32_t HeaderTable::toExternal(uint32_t internalIndex) const {
  int _state = 0;
  while (true) {
    switch (_state) {
      case 0:
        return toExternal(head_, table_.size(), internalIndex);
    }
  }
}

uint32_t HeaderTable::toExternal(uint32_t head, uint32_t length,
                                 uint32_t internalIndex) {
  int _state = 0;
  while (true) {
    switch (_state) {
      case 0:
        return ((head + length - internalIndex) % length) + 1;
    }
  }
}

uint32_t HeaderTable::toInternal(uint32_t externalIndex) const {
  int _state = 0;
  while (true) {
    switch (_state) {
      case 0:
        return toInternal(head_, table_.size(), externalIndex);
    }
  }
}

uint32_t HeaderTable::toInternal(uint32_t head, uint32_t length,
                                 uint32_t externalIndex) {
  int _state = 0;
  while (true) {
    switch (_state) {
      case 0:
        --externalIndex;
        return (head + length - externalIndex) % length;
    }
  }
}

bool HeaderTable::operator==(const HeaderTable& other) const {
  int _state = 0;
  while (true) {
    switch (_state) {
      case 0:
        if (size() != other.size()) {
          return false;
        }
        _state = 1;
        break;
      case 1:
        if (bytes() != other.bytes()) {
          return false;
        }
        _state = 2;
        break;
      case 2: {
        list<uint32_t> refset = referenceSet();
        refset.sort();
        list<uint32_t> otherRefset = other.referenceSet();
        otherRefset.sort();
        if (refset != otherRefset) {
          return false;
        }
        return true;
      }
    }
  }
}

std::ostream& operator<<(std::ostream& os, const HeaderTable& table) {
  int _state = 0;
  while (true) {
    switch (_state) {
      case 0:
        os << std::endl;
        for (size_t i = 1; i <= table.size(); i++) {
          const HPACKHeader& h = table[i];
          os << '[' << i << "] (s=" << h.bytes() << ") " << h.name << ": " << h.value << std::endl;
        }
        _state = 1;
        break;
      case 1:
        os << "reference set: [";
        for (const auto& index : table.referenceSet()) {
          os << index << ", ";
        }
        _state = 2;
        break;
      case 2:
        os << "]" << std::endl;
        os << "total size: " << table.bytes() << std::endl;
        return os;
    }
  }
}

}