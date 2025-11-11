pragma solidity ^0.4.19;

contract MONEY_BOX {
    struct Holder {
        uint a;
        uint b;
    }
    
    mapping (address => Holder) public c;
    
    uint public d;
    
    Log e;
    
    bool f;
    
    function SetMinSum(uint g) public {
        if(f)throw;
        d = g;
    }
    
    function SetLogFile(address h) public {
        if(f)throw;
        e = Log(h);
    }
    
    function Initialized() public {
        f = true;
    }
    
    function Put(uint i) public payable {
        var j = c[msg.sender];
        j.b += msg.value;
        if(now+i > j.a) j.a = now + i;
        e.AddMessage(msg.sender, msg.value, "Put");
    }
    
    function Collect(uint k) public payable {
        var l = c[msg.sender];
        if(l.b >= d && l.b >= k && now > l.a) {
            if(msg.sender.call.value(k)()) {
                l.b -= k;
                e.AddMessage(msg.sender, k, "Collect");
            }
        }
    }
    
    function() public payable {
        Put(0);
    }
}

contract Log {
    struct Message {
        address m;
        string n;
        uint o;
        uint p;
    }
    
    Message[] public q;
    
    Message r;
    
    function AddMessage(address s, uint t, string u) public {
        r.m = s;
        r.p = now;
        r.o = t;
        r.n = u;
        q.push(r);
    }
}