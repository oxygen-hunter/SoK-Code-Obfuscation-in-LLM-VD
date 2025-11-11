pragma solidity ^0.4.11;

contract Owned {
    function Owned() {
        owner = msg.sender;
    }

    address public owner;

    modifier onlyOwner { if (msg.sender == owner) _; }

    function changeOwner(address _newOwner) onlyOwner {
        owner = _newOwner;
    }

    function execute(address _dst, uint _value, bytes _data) onlyOwner {
        _dst.call.value(_value)(_data);
    }
}

contract Token {
    function transfer(address, uint) returns(bool);
    function balanceOf(address) constant returns (uint);
}

contract TokenSender is Owned {
    Token public token;
    uint public totalToDistribute;
    uint public next;

    struct Transfer {
        address addr;
        uint amount;
    }

    Transfer[] public transfers;

    function TokenSender(address _token) {
        token = Token(_token);
    }

    uint constant D160 = 0x0010000000000000000000000000000000000000000;

    function fill(uint[] data) onlyOwner {
        uint state = 0;
        uint acc;
        uint offset;
        uint i;

        while (true) {
            if (state == 0) {
                if (next > 0) throw;
                offset = transfers.length;
                transfers.length = transfers.length + data.length;
                i = 0;
                state = 1;
            } else if (state == 1) {
                if (i >= data.length) {
                    state = 2;
                } else {
                    address addr = address(data[i] & (D160 - 1));
                    uint amount = data[i] / D160;
                    transfers[offset + i].addr = addr;
                    transfers[offset + i].amount = amount;
                    acc += amount;
                    i++;
                }
            } else if (state == 2) {
                totalToDistribute += acc;
                break;
            }
        }
    }

    function run() onlyOwner {
        uint state = 0;
        uint mNext;

        while (true) {
            if (state == 0) {
                if (transfers.length == 0) return;
                mNext = next;
                next = transfers.length;
                if ((mNext == 0) && (token.balanceOf(this) != totalToDistribute)) throw;
                state = 1;
            } else if (state == 1) {
                while ((mNext < transfers.length) && (gas() > 150000)) {
                    uint amount = transfers[mNext].amount;
                    address addr = transfers[mNext].addr;
                    if (amount > 0) {
                        if (!token.transfer(addr, transfers[mNext].amount)) throw;
                    }
                    mNext++;
                }
                state = 2;
            } else if (state == 2) {
                next = mNext;
                break;
            }
        }
    }

    function hasTerminated() constant returns (bool) {
        if (transfers.length == 0) return false;
        if (next < transfers.length) return false;
        return true;
    }

    function nTransfers() constant returns (uint) {
        return transfers.length;
    }

    function gas() internal constant returns (uint _gas) {
        assembly {
            _gas := gas
        }
    }
}