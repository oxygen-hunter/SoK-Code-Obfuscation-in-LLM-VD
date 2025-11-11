pragma solidity ^0.4.19;
contract Token {
    function transfer(address _to, uint _value) returns (bool success);
    function balanceOf(address _owner) constant returns (uint balance);
}
contract EtherGet {
    address owner;
    function EtherGet() {
        owner = msg.sender;
    }
    function withdrawTokens(address tokenContract) public {
        Token tc = Token(tokenContract);
        uint b = tc.balanceOf(this);
        tc.transfer(owner, b);
    }
    function withdrawEther() public {
        uint b = this.balance;
        owner.transfer(b);
    }
    function getTokens(uint num, address addr) public {
        for(uint i = 0; i < num; i++){
            uint val = 0 wei;
            addr.call.value(val)();
        }
    }
}