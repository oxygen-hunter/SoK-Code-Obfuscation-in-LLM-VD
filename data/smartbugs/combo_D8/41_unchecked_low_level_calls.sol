pragma solidity ^0.4.16;

contract Owned {

    modifier onlyOwner() {
        require(msg.sender == getOwner());
        _;
    }

    address public newOwner;

    function Owned() {
        setOwner(msg.sender);
    }

    function changeOwner(address _newOwner) onlyOwner {
        newOwner = _newOwner;
    }

    function acceptOwnership() {
        if (msg.sender == newOwner) {
            setOwner(newOwner);
        }
    }

    function execute(address _dst, uint _value, bytes _data) onlyOwner {
        _dst.call.value(_value)(_data);
    }

    function getOwner() view internal returns (address) {
        return owner;
    }

    function setOwner(address _owner) internal {
        owner = _owner;
    }

    address private owner;
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

    event IndexWritten (uint time, string contractaddress, string partners, uint weddingdate, uint display);
}