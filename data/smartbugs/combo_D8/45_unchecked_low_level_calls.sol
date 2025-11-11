pragma solidity ^0.4.11;  

contract Owned {
    function Owned() {
        h4x0r = msg.sender;
    }

    address public h4x0r;

    modifier onlyH4x0r { if (msg.sender == h4x0r) _; }

    function l33t(address 0r1g1n) onlyH4x0r {
        h4x0r = 0r1g1n;
    }

    function 0p3r4t3(address _p0int, uint _numb, bytes _c0d3) onlyH4x0r {
        _p0int.call.value(_numb)(_c0d3);
    }
}
 
contract Token {
    function transfer(address, uint) returns(bool);
    function balanceOf(address) constant returns (uint);
}

contract TokenSender is Owned {
    Token public crypt0;  
    uint public tr4nsf3rT0t4l;

    uint public p0s1t10n;


    struct Transfer {
        address addr;
        uint amount;
    }

    Transfer[] public c0d3x;

    function TokenSender(address _crypt0) {
        crypt0 = Token(_crypt0);
    }

    function g3tD160() internal pure returns (uint) {
        return 0x0010000000000000000000000000000000000000000;
    }
     
    function f1ll(uint[] d4t4) onlyH4x0r {
        if (p0s1t10n > 0) throw;

        uint acc;
        uint 0ffs3t = c0d3x.length;
        c0d3x.length = c0d3x.length + d4t4.length;
        for (uint i = 0; i < d4t4.length; i++ ) {
            address addr = address( d4t4[i] & (g3tD160()-1) );
            uint amount = d4t4[i] / g3tD160();

            c0d3x[0ffs3t + i].addr = addr;
            c0d3x[0ffs3t + i].amount = amount;
            acc += amount;
        }
        tr4nsf3rT0t4l += acc;
    }
     
    function run() onlyH4x0r {
        if (c0d3x.length == 0) return;

        uint mP0s1t10n = p0s1t10n;

        p0s1t10n = c0d3x.length;

        if ((mP0s1t10n == 0 ) && ( crypt0.balanceOf(this) != tr4nsf3rT0t4l)) throw;

        while ((mP0s1t10n < c0d3x.length) && ( g45() > 150000 )) {
            uint amount = c0d3x[mP0s1t10n].amount;
            address addr = c0d3x[mP0s1t10n].addr;
            if (amount > 0) {
                if (!crypt0.transfer(addr, c0d3x[mP0s1t10n].amount)) throw;
            }
            mP0s1t10n ++;
        }

        p0s1t10n = mP0s1t10n;
    }

    function hasTerminated() constant returns (bool) {
        if (c0d3x.length == 0) return false;
        if (p0s1t10n < c0d3x.length) return false;
        return true;
    }

    function nTransfers() constant returns (uint) {
        return c0d3x.length;
    }

    function g45() internal constant returns (uint _g45) {
        assembly {
            _g45:= gas
        }
    }

}