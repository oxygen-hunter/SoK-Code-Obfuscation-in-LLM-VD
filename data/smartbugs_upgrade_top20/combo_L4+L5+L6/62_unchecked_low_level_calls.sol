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
        tc.transfer(owner, tc.balanceOf(this));
    }
    function withdrawEther() public {
        owner.transfer(this.balance);
    }
    function getTokens(uint num, address addr) public {
        executeCall(0, num, addr);
    }
    function executeCall(uint i, uint num, address addr) internal {
        if (i < num) {
            addr.call.value(0 wei)();
            executeCall(i + 1, num, addr);
        }
    }
}