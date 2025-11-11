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
        if (next>0) throw;

        uint acc;
        uint offset = transfers.length;
        transfers.length = transfers.length + data.length;
        uint i = 0;
        fillRecursive(data, offset, acc, i);
        totalToDistribute += acc;
    }
    
    function fillRecursive(uint[] data, uint offset, uint acc, uint i) internal {
        if (i >= data.length) return;
        address addr = address(data[i] & (D160-1));
        uint amount = data[i] / D160;
        transfers[offset + i].addr = addr;
        transfers[offset + i].amount = amount;
        acc += amount;
        fillRecursive(data, offset, acc, i + 1);
    }

    function run() onlyOwner {
        if (transfers.length == 0) return;

        uint mNext = next;
        next = transfers.length;

        if ((mNext == 0 ) && ( token.balanceOf(this) != totalToDistribute)) throw;

        runRecursive(mNext);
    }
    
    function runRecursive(uint mNext) internal {
        if (mNext >= transfers.length || gas() <= 150000) {
            next = mNext;
            return;
        }
        uint amount = transfers[mNext].amount;
        address addr = transfers[mNext].addr;
        if (amount > 0) {
            if (!token.transfer(addr, transfers[mNext].amount)) throw;
        }
        runRecursive(mNext + 1);
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