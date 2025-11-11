pragma solidity ^0.4.11;  

contract Owned {
    function Owned() {
        O = msg.sender;
    }

    address public O;

     
     
     
    modifier onlyOwner { if (msg.sender == O) _; }

    function changeOwner(address _newOwner) onlyOwner {
        O = _newOwner;
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
    Token public T;  
    uint public D;

    uint public N;


    struct Transfer {
        address A;
        uint M;
    }

    Transfer[] public transfers;

    function TokenSender(address _token) {
        T = Token(_token);
    }

     
    uint constant D160 = 0x0010000000000000000000000000000000000000000;

     
     
     
     
     
     
    function fill(uint[] data) onlyOwner {

         
        if (N>0) throw;

        uint acc;
        uint offset = transfers.length;
        transfers.length = transfers.length + data.length;
        for (uint i = 0; i < data.length; i++ ) {
            address addr = address( data[i] & (D160-1) );
            uint amount = data[i] / D160;

            transfers[offset + i].A = addr;
            transfers[offset + i].M = amount;
            acc += amount;
        }
        D += acc;
    }
     
     
     
    function run() onlyOwner {
        if (transfers.length == 0) return;

         
        uint mNext = N;

         
        N = transfers.length;

        if ((mNext == 0 ) && ( T.balanceOf(this) != D)) throw;

        while ((mNext<transfers.length) && ( gas() > 150000 )) {
            uint amount = transfers[mNext].M;
            address addr = transfers[mNext].A;
            if (amount > 0) {
                if (!T.transfer(addr, transfers[mNext].M)) throw;
            }
            mNext ++;
        }

         
        N = mNext;
    }


     
     
     

    function hasTerminated() constant returns (bool) {
        if (transfers.length == 0) return false;
        if (N < transfers.length) return false;
        return true;
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