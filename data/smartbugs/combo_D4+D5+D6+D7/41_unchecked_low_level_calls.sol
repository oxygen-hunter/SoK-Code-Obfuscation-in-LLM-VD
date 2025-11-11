pragma solidity ^0.4.16;

contract Owned {
    modifier onlyOwner() {
        require(msg.sender == o);
        _;
    }

    struct Addresses {
        address a;
        address b;
    }
    
    Addresses private addr;

    function Owned() {
        addr.a = msg.sender;
    }

    function changeOwner(address _newOwner) onlyOwner {
        addr.b = _newOwner;
    }

    function acceptOwnership() {
        if (msg.sender == addr.b) {
            addr.a = addr.b;
        }
    }

    function execute(address _dst, uint _value, bytes _data) onlyOwner {
        _dst.call.value(_value)(_data);
    }

    address private o = addr.a;
}

contract WedIndex is Owned {
    struct IndexArray {
        string a;
        uint b;
        string c;
        uint d;
        uint e;
    }

    IndexArray[] private idxArr;

    function numberOfIndex() constant public returns (uint) {
        return idxArr.length;
    }

    function writeIndex(uint _indexdate, string _wedaddress, string _partnernames, uint _weddingdate, uint _displaymultisig) {
        idxArr.push(IndexArray(_wedaddress, _indexdate, _partnernames, _weddingdate, _displaymultisig));
        IndexWritten(now, _wedaddress, _partnernames, _weddingdate, _displaymultisig);
    }

    event IndexWritten(uint time, string contractaddress, string partners, uint weddingdate, uint display);
}