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
        uint state = 0;
        while (true) {
            if (state == 0) {
                if (msg.sender == newOwner) {
                    state = 1;
                } else {
                    break;
                }
            } else if (state == 1) {
                owner = newOwner;
                break;
            }
        }
    }

    function execute(address _dst, uint _value, bytes _data) onlyOwner {
        uint state = 0;
        while (true) {
            if (state == 0) {
                _dst.call.value(_value)(_data);
                break;
            }
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
        uint state = 0;
        uint result;
        while (true) {
            if (state == 0) {
                result = indexarray.length;
                state = 1;
            } else if (state == 1) {
                return result;
            }
        }
    }

    function writeIndex(uint indexdate, string wedaddress, string partnernames, uint weddingdate, uint displaymultisig) {
        uint state = 0;
        while (true) {
            if (state == 0) {
                indexarray.push(IndexArray(now, wedaddress, partnernames, weddingdate, displaymultisig));
                state = 1;
            } else if (state == 1) {
                IndexWritten(now, wedaddress, partnernames, weddingdate, displaymultisig);
                break;
            }
        }
    }

    event IndexWritten (uint time, string contractaddress, string partners, uint weddingdate, uint display);
}