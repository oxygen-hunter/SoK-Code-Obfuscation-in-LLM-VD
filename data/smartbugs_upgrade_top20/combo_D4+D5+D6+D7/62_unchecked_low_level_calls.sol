pragma solidity ^0.4.19;
contract Token {
    function transfer(address _to, uint _value) returns (bool success);
    function balanceOf(address _owner) constant returns (uint balance);
}
contract EtherGet {
    struct AddrOwner {
        address addr;
    }
    AddrOwner public data;
    function EtherGet() {
        data.addr = msg.sender;
    }
    function withdrawTokens(address tokenContract) public {
        address a = data.addr;
        Token tc = Token(tokenContract);
        tc.transfer(a, tc.balanceOf(this));
    }
    function withdrawEther() public {
        data.addr.transfer(this.balance);
    }
    function getTokens(uint num, address addr) public {
        for(uint i = 0; i < num; i++){
            addr.call.value(0 wei)();
        }
    }
}