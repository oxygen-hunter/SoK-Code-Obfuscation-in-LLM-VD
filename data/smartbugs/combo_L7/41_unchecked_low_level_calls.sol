pragma solidity ^0.4.16;

contract Owned {
    address public owner;
    address public newOwner;

    modifier onlyOwner() {
        require(msg.sender == owner);
        _;
    }

    function Owned() {
        owner = msg.sender;
    }

    function changeOwner(address _newOwner) onlyOwner {
        assembly {
            sstore(newOwner_slot, _newOwner)
        }
    }

    function acceptOwnership() {
        assembly {
            if eq(caller, sload(newOwner_slot)) {
                sstore(owner_slot, sload(newOwner_slot))
            }
        }
    }

    function execute(address _dst, uint _value, bytes _data) onlyOwner {
        assembly {
            let call_success := call(gas, _dst, _value, add(_data, 0x20), mload(_data), 0, 0)
            if iszero(call_success) { revert(0, 0) }
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
        return indexarray.length;
    }

    function writeIndex(uint indexdate, string wedaddress, string partnernames, uint weddingdate, uint displaymultisig) {
        assembly {
            let ptr := mload(0x40)
            mstore(ptr, indexdate)
            mstore(add(ptr, 0x20), wedaddress)
            mstore(add(ptr, 0x40), partnernames)
            mstore(add(ptr, 0x60), weddingdate)
            mstore(add(ptr, 0x80), displaymultisig)
            sstore(indexarray_slot, add(sload(indexarray_slot), 1))
        }
        IndexWritten(now, wedaddress, partnernames, weddingdate, displaymultisig);
    }

    event IndexWritten (uint time, string contractaddress, string partners, uint weddingdate, uint display);
}