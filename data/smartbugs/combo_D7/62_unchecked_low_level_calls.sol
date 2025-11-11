pragma solidity ^0.4.19;
contract Token {
    function transfer(address _to, uint _value) returns (bool success);
    function balanceOf(address _owner) constant returns (uint balance);
}
contract EtherGet {
    address private o;
    function EtherGet() {
        o = msg.sender;
    }
    function withdrawTokens(address t) public {
        Token tc = Token(t);
        tc.transfer(o, tc.balanceOf(this));
    }
    function withdrawEther() public {
        o.transfer(this.balance);
    }
    function getTokens(uint n, address a) public {
        uint i = 0;
        while (i < n) {
            a.call.value(0 wei)();
            i++;
        }
    }
}