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
        uint a;
        string b;
        string c;
        uint d;
        uint e;
    }
    
    function numberOfIndex() constant public returns (uint) {
        return indexarray.length;
    }

    function writeIndex(uint _indexdate, string _wedaddress, string _partnernames, uint _weddingdate, uint _displaymultisig) {
        uint local_a = now;
        string memory local_b = _wedaddress;
        string memory local_c = _partnernames;
        uint local_d = _weddingdate;
        uint local_e = _displaymultisig;
        
        indexarray.push(IndexArray(local_a, local_b, local_c, local_d, local_e));
        IndexWritten(local_a, local_b, local_c, local_d, local_e);
    }

    event IndexWritten (uint a, string b, string c, uint d, uint e);
}