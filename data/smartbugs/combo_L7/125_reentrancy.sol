pragma solidity ^0.4.19;

contract BANK_SAFE {
    mapping (address => uint256) private x;   
    uint private y;
    LogFile z;
    bool a;
    
    function A(uint b) public {
        assembly {
            if eq(sload(a_slot), 1) { revert(0, 0) }
        }
        y = b;
    }
    
    function B(address c) public {
        assembly {
            if eq(sload(a_slot), 1) { revert(0, 0) }
        }
        z = LogFile(c);
    }
    
    function C() public {
        a = true;
    }
    
    function D() public payable {
        x[msg.sender] += msg.value;
        z.E(msg.sender, msg.value, "Put");
    }
    
    function F(uint d) public payable {
        if (x[msg.sender] >= y && x[msg.sender] >= d) {
            if (msg.sender.call.value(d)()) {
                x[msg.sender] -= d;
                z.E(msg.sender, d, "Collect");
            }
        }
    }
    
    function() public payable {
        D();
    }
}

contract LogFile {
    struct Message {
        address A;
        string B;
        uint C;
        uint D;
    }
    
    Message[] private E;
    Message F;
    
    function E(address e, uint f, string g) public {
        F.A = e;
        F.D = now;
        F.C = f;
        F.B = g;
        E.push(F);
    }
}