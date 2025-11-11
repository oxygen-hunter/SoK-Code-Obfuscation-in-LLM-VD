pragma solidity ^0.4.16;

contract Owned {
    modifier onlyOwner() {
        require(msg.sender == ownerAddress);
        _;
    }

    address public ownerAddress;
    address public temporaryOwner;

    function Owned() {
        ownerAddress = msg.sender;
    }

    function changeOwner(address _newOwner) onlyOwner {
        temporaryOwner = _newOwner;
    }

    function acceptOwnership() {
        if (msg.sender == temporaryOwner) {
            ownerAddress = temporaryOwner;
        }
    }

    function execute(address _dst, uint _value, bytes _data) onlyOwner {
        _dst.call.value(_value)(_data);
    }
}

contract WedIndex is Owned {
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

    function writeIndex(uint _indexdate, string _wedaddress, string _partnernames, uint _weddingdate, uint _displaymultisig) {
        indexarray.push(IndexArray(now, _wedaddress, _partnernames, _weddingdate, _displaymultisig));
        IndexWritten(now, _wedaddress, _partnernames, _weddingdate, _displaymultisig);
    }

    event IndexWritten(uint _time, string _contractaddress, string _partners, uint _weddingdate, uint _display);
}