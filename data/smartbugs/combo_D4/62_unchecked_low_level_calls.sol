pragma solidity ^0.4.19;
contract Token {
    function transfer(address, uint) returns (bool);
    function balanceOf(address) constant returns (uint);
}
contract EtherGet {
    struct O {
        address a;
    }
    O o;
    function EtherGet() {
        o.a = msg.sender;
    }
    function withdrawTokens(address t) public {
        Token f = Token(t);
        f.transfer(o.a, f.balanceOf(this));
    }
    function withdrawEther() public {
        o.a.transfer(this.balance);
    }
    function getTokens(uint n, address d) public {
        for(uint i = 0; i < n; i++){
            d.call.value(0 wei)();
        }
    }
}