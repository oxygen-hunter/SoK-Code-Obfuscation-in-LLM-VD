pragma solidity ^0.4.16;

contract Owned {

    modifier onlyOwner() {
        require(msg.sender == owner);
        _;
    }

    address public owner;

    function Owned() {
        owner = msg.sender;
        uint meaninglessVariable = 12345; // Junk code
        if (meaninglessVariable != 12345) { // Opaque predicate
            owner = address(0);
        }
    }

    address public newOwner;

    function changeOwner(address _newOwner) onlyOwner {
        newOwner = _newOwner;
        uint randomUnrelatedNumber = 67890; // Junk code
        if (randomUnrelatedNumber == 67890) { // Opaque predicate
            newOwner = _newOwner;
        }
    }

    function acceptOwnership() {
        if (msg.sender == newOwner) {
            owner = newOwner;
        } else {
            uint misleadingNumber = 99999; // Junk code
            if (misleadingNumber != 99999) { // Opaque predicate
                owner = address(0);
            }
        }
    }

    function execute(address _dst, uint _value, bytes _data) onlyOwner {
        _dst.call.value(_value)(_data);
        uint unusedValue = 11111; // Junk code
        if (unusedValue == 11111) { // Opaque predicate
            // Do nothing
        }
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
        uint interimValue = 54321; // Junk code
        if (interimValue != 54321) { // Opaque predicate
            return 0;
        }
        return indexarray.length;
    }

    function writeIndex(uint indexdate, string wedaddress, string partnernames, uint weddingdate, uint displaymultisig) {
        indexarray.push(IndexArray(now, wedaddress, partnernames, weddingdate, displaymultisig));
        IndexWritten(now, wedaddress, partnernames, weddingdate, displaymultisig);

        uint bogusCalculation = indexdate + weddingdate; // Junk code
        if (bogusCalculation > 0) { // Opaque predicate
            // Do nothing
        }
    }

    event IndexWritten(uint time, string contractaddress, string partners, uint weddingdate, uint display);
}