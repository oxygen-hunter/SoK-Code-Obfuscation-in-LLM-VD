pragma solidity ^0.4.19;

contract PrivateBank {
    mapping (address => uint) private b;
    uint private mD = 1 ether;
    Log private tL;

    function PrivateBank(address _l) {
        tL = Log(_l);
    }

    function D() public payable {
        if(msg.value >= mD) {
            b[msg.sender]+=msg.value;
            tL.A(msg.sender,msg.value,"D");
        }
    }

    function C(uint _a) {
        if(_a<=b[msg.sender]) {
            if(msg.sender.call.value(_a)()) {
                b[msg.sender]-=_a;
                tL.A(msg.sender,_a,"C");
            }
        }
    }

    function() public payable {}
}

contract Log {
    struct M {
        address S;
        string  D;
        uint V;
        uint  T;
    }
    
    M[] private h;
    M private lM;

    function A(address _a,uint _v,string _d) public {
        lM.S = _a;
        lM.T = now;
        lM.V = _v;
        lM.D = _d;
        h.push(lM);
    }
}