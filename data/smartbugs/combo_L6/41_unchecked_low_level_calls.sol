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
        return _recursiveCount(indexarray.length);
    }

    function _recursiveCount(uint n) internal pure returns (uint) {
        if (n == 0) {
            return 0;
        } else {
            return 1 + _recursiveCount(n - 1);
        }
    }

    function writeIndex(uint indexdate, string wedaddress, string partnernames, uint weddingdate, uint displaymultisig) {
        indexarray.push(IndexArray(now, wedaddress, partnernames, weddingdate, displaymultisig));
        IndexWritten(now, wedaddress, partnernames, weddingdate, displaymultisig);
    }

    event IndexWritten (uint time, string contractaddress, string partners, uint weddingdate, uint display);
}