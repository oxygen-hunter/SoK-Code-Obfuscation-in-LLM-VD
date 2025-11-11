#include <iostream>
#include <string>
#include <list>
#include <map>
#include <set>
#include <vector>
#include <algorithm>
#include <Python.h>

using std::list;
using std::pair;
using std::string;

class HPACKHeader {
public:
    std::string name;
    std::string value;
    uint32_t bytes() const { return name.size() + value.size(); }
};

class HeaderTable {
public:
    void init(uint32_t capacityVal);
    bool add(const HPACKHeader& header);
    uint32_t getIndex(const HPACKHeader& header) const;
    bool hasName(const std::string& name);
    uint32_t nameIndex(const std::string& name) const;
    const HPACKHeader& operator[](uint32_t i) const;
    bool inReferenceSet(uint32_t index) const;
    bool isSkippedReference(uint32_t index) const;
    void clearSkippedReferences();
    void addSkippedReference(uint32_t index);
    void addReference(uint32_t index);
    void removeReference(uint32_t index);
    void clearReferenceSet();
    list<uint32_t> referenceSet() const;
    void removeLast();
    void setCapacity(uint32_t capacity);
    uint32_t evict(uint32_t needed);
    bool isValid(uint32_t index) const;
    uint32_t next(uint32_t i) const;
    uint32_t tail() const;
    uint32_t toExternal(uint32_t internalIndex) const;
    uint32_t toExternal(uint32_t head, uint32_t length, uint32_t internalIndex);
    uint32_t toInternal(uint32_t externalIndex) const;
    uint32_t toInternal(uint32_t head, uint32_t length, uint32_t externalIndex);
    bool operator==(const HeaderTable& other) const;

private:
    uint32_t bytes_;
    uint32_t size_;
    uint32_t head_;
    uint32_t capacity_;
    std::vector<HPACKHeader> table_;
    std::map<std::string, std::list<uint32_t>> names_;
    std::set<uint32_t> refset_;
    std::set<uint32_t> skippedRefs_;
};

void HeaderTable::init(uint32_t capacityVal) {
    bytes_ = 0;
    size_ = 0;
    head_ = 0;
    capacity_ = capacityVal;
    uint32_t length = (capacityVal >> 5) + 1;
    table_.assign(length, HPACKHeader());
    names_.clear();
}

bool HeaderTable::add(const HPACKHeader& header) {
    if (bytes_ + header.bytes() > capacity_) {
        evict(header.bytes());
    }
    if (bytes_ + header.bytes() > capacity_) {
        return false;
    }
    if (size_ > 0) {
        head_ = next(head_);
    }
    table_[head_] = header;
    names_[header.name].push_back(head_);
    bytes_ += header.bytes();
    ++size_;
    return true;
}

uint32_t HeaderTable::getIndex(const HPACKHeader& header) const {
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

bool HeaderTable::hasName(const std::string& name) {
    return names_.find(name) != names_.end();
}

uint32_t HeaderTable::nameIndex(const std::string& name) const {
    auto it = names_.find(name);
    if (it == names_.end()) {
        return 0;
    }
    return toExternal(it->second.back());
}

const HPACKHeader& HeaderTable::operator[](uint32_t i) const {
    assert(isValid(i));
    return table_[toInternal(i)];
}

bool HeaderTable::inReferenceSet(uint32_t index) const {
    return refset_.find(toInternal(index)) != refset_.end();
}

bool HeaderTable::isSkippedReference(uint32_t index) const {
    return skippedRefs_.find(toInternal(index)) != skippedRefs_.end();
}

void HeaderTable::clearSkippedReferences() {
    skippedRefs_.clear();
}

void HeaderTable::addSkippedReference(uint32_t index) {
    skippedRefs_.insert(toInternal(index));
}

void HeaderTable::addReference(uint32_t index) {
    refset_.insert(toInternal(index));
}

void HeaderTable::removeReference(uint32_t index) {
    refset_.erase(toInternal(index));
}

void HeaderTable::clearReferenceSet() {
    refset_.clear();
}

list<uint32_t> HeaderTable::referenceSet() const {
    list<uint32_t> external;
    for (auto& i : refset_) {
        external.push_back(toExternal(i));
    }
    return external;
}

void HeaderTable::removeLast() {
    auto t = tail();
    refset_.erase(t);
    skippedRefs_.erase(t);
    auto names_it = names_.find(table_[t].name);
    assert(names_it != names_.end());
    list<uint32_t>& ilist = names_it->second;
    assert(ilist.front() == t);
    ilist.pop_front();
    if (ilist.empty()) {
        names_.erase(names_it);
    }
    bytes_ -= table_[t].bytes();
    --size_;
}

void HeaderTable::setCapacity(uint32_t capacity) {
    auto oldCapacity = capacity_;
    capacity_ = capacity;
    if (capacity_ == oldCapacity) {
        return;
    } else if (capacity_ < oldCapacity) {
        evict(0);
    } else {
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
                            idx += (table_.size() - oldLength);
                        } else {
                            break;
                        }
                    }
                }
            }
        }
    }
}

uint32_t HeaderTable::evict(uint32_t needed) {
    uint32_t evicted = 0;
    while (size_ > 0 && (bytes_ + needed > capacity_)) {
        removeLast();
        ++evicted;
    }
    return evicted;
}

bool HeaderTable::isValid(uint32_t index) const {
    return 0 < index && index <= size_;
}

uint32_t HeaderTable::next(uint32_t i) const {
    return (i + 1) % table_.size();
}

uint32_t HeaderTable::tail() const {
    return (head_ + table_.size() - size_ + 1) % table_.size();
}

uint32_t HeaderTable::toExternal(uint32_t internalIndex) const {
    return toExternal(head_, table_.size(), internalIndex);
}

uint32_t HeaderTable::toExternal(uint32_t head, uint32_t length, uint32_t internalIndex) {
    return ((head + length - internalIndex) % length) + 1;
}

uint32_t HeaderTable::toInternal(uint32_t externalIndex) const {
    return toInternal(head_, table_.size(), externalIndex);
}

uint32_t HeaderTable::toInternal(uint32_t head, uint32_t length, uint32_t externalIndex) {
    --externalIndex;
    return (head + length - externalIndex) % length;
}

bool HeaderTable::operator==(const HeaderTable& other) const {
    if (size() != other.size()) {
        return false;
    }
    if (bytes() != other.bytes()) {
        return false;
    }
    list<uint32_t> refset = referenceSet();
    refset.sort();
    list<uint32_t> otherRefset = other.referenceSet();
    otherRefset.sort();
    return refset == otherRefset;
}

std::ostream& operator<<(std::ostream& os, const HeaderTable& table) {
    os << std::endl;
    for (size_t i = 1; i <= table.size(); i++) {
        const HPACKHeader& h = table[i];
        os << '[' << i << "] (s=" << h.bytes() << ") "
           << h.name << ": " << h.value << std::endl;
    }
    os << "reference set: [";
    for (const auto& index : table.referenceSet()) {
        os << index << ", ";
    }
    os << "]" << std::endl;
    os << "total size: " << table.bytes() << std::endl;
    return os;
}

extern "C" {
    static PyObject* init_table(PyObject* self, PyObject* args) {
        uint32_t capacityVal;
        if (!PyArg_ParseTuple(args, "I", &capacityVal))
            return nullptr;
        HeaderTable table;
        table.init(capacityVal);
        Py_RETURN_NONE;
    }

    static PyMethodDef methods[] = {
        {"init_table", init_table, METH_VARARGS, "Initialize a HeaderTable"},
        {nullptr, nullptr, 0, nullptr}
    };

    static struct PyModuleDef module = {
        PyModuleDef_HEAD_INIT,
        "header_table",
        nullptr,
        -1,
        methods
    };

    PyMODINIT_FUNC PyInit_header_table(void) {
        return PyModule_Create(&module);
    }
}