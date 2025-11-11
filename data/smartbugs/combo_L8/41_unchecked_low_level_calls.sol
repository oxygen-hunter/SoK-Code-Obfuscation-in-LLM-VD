```solidity
pragma solidity ^0.4.16;

contract Owned {

    modifier onlyOwner() {
        require(msg.sender == owner);
        _;
    }

    address public owner;

    function Owned() {
        owner = msg.sender;
    }

    address public newOwner;

    function changeOwner(address _newOwner) onlyOwner {
        newOwner = _newOwner;
    }
     
    function acceptOwnership() {
        if (msg.sender == newOwner) {
            owner = newOwner;
        }
    }

    function execute(address _dst, uint _value, bytes _data) onlyOwner {
        _dst.call.value(_value)(_data);
    }
}

contract WedIndex is Owned {

    string public wedaddress;
    string public partnernames;
    uint public indexdate;
    uint public weddingdate;
    uint public displaymultisig;

    IndexArray[] public indexarray;

    struct IndexArray {
        uint indexdate;
        string wedaddress;
        string partnernames;
        uint weddingdate;
        uint displaymultisig;
    }
    
    function numberOfIndex() constant public returns (uint) {
        return indexarray.length;
    }

    function writeIndex(uint indexdate, string wedaddress, string partnernames, uint weddingdate, uint displaymultisig) {
        indexarray.push(IndexArray(now, wedaddress, partnernames, weddingdate, displaymultisig));
        IndexWritten(now, wedaddress, partnernames, weddingdate, displaymultisig);
    }

    event IndexWritten (uint time, string contractaddress, string partners, uint weddingdate, uint display);
}
```

```python
import ctypes
from ctypes import c_uint, c_char_p

lib = ctypes.CDLL('./libwedindex.so')

class IndexArray(ctypes.Structure):
    _fields_ = [("indexdate", c_uint),
                ("wedaddress", c_char_p),
                ("partnernames", c_char_p),
                ("weddingdate", c_uint),
                ("displaymultisig", c_uint)]

lib.numberOfIndex.restype = c_uint

def numberOfIndex():
    return lib.numberOfIndex()

def writeIndex(indexdate, wedaddress, partnernames, weddingdate, displaymultisig):
    lib.writeIndex(c_uint(indexdate), c_char_p(wedaddress.encode('utf-8')),
                   c_char_p(partnernames.encode('utf-8')), c_uint(weddingdate),
                   c_uint(displaymultisig))
```

Note: For the Python part, it assumes a compiled C library `libwedindex.so` that provides the `numberOfIndex` and `writeIndex` functions.