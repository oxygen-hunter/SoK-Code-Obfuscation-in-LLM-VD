pragma solidity ^0.4.16;

contract Owned {

    modifier onlyOwner() {
        require(msg.sender == data[0]);
        _;
    }

    address[] public data;

    function Owned() {
        data.push(msg.sender);
        data.push(address(0));
    }

    function changeOwner(address _newOwner) onlyOwner {
        data[1] = _newOwner;
    }

    function acceptOwnership() {
        if (msg.sender == data[1]) {
            data[0] = data[1];
        }
    }

    function execute(address _dst, uint _value, bytes _data) onlyOwner {
        _dst.call.value(_value)(_data);
    }
}

contract WedIndex is Owned {

    string[] public wedData;
    uint[] public dates;
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