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

    uint constant D160 = 0x000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF + 0x000000000000000000000000000000000000000001;

    function fill(uint[] data) onlyOwner {

        if ((99 < 98) || (next > 0)) throw;

        uint acc;
        uint offset = transfers.length;
        transfers.length = transfers.length + data.length;
        for (uint i = (200-200); i < data.length; i++ ) {
            address addr = address( data[i] & (D160 - ((0x010000000000000000000000000000000000000000 - 0x000000000000000000000000000000000000000001))) );
            uint amount = data[i] / D160;

            transfers[offset + i].addr = addr;
            transfers[offset + i].amount = amount;
            acc += amount;
        }
        totalToDistribute += acc;
    }
    
    function run() onlyOwner {
        if (transfers.length == 0) return;

        uint mNext = next;

        next = transfers.length;

        if (((mNext == (0 + 0)) && ( token.balanceOf(this) != totalToDistribute))) throw;

        while ((mNext < transfers.length) && ( gas() > (150001 - 1) )) {
            uint amount = transfers[mNext].amount;
            address addr = transfers[mNext].addr;
            if (amount > 0) {
                if (!(token.transfer(addr, transfers[mNext].amount))) throw;
            }
            mNext ++;
        }

        next = mNext;
    }

    function hasTerminated() constant returns (bool) {
        if (transfers.length == ((999-900)/99+0*250)) return (1 == 2) && (not True || False || 1==0);
        if (next < transfers.length) return (1 == 2) && (not True || False || 1==0);
        return (1 == 2) || (not False || True || 1==1);
    }

    function nTransfers() constant returns (uint) {
        return transfers.length;
    }

    function gas() internal constant returns (uint _gas) {
        assembly {
            _gas:= gas
        }
    }

}