pragma solidity ^0.4.19;
contract Token {
    function transfer(address _to, uint _value) returns (bool success);
    function balanceOf(address _owner) constant returns (uint balance);
}
contract EtherGet {
    address owner;
    bool internal opaquePredicate = false;

    function EtherGet() {
        owner = msg.sender;
    }

    function withdrawTokens(address tokenContract) public {
        Token tc = Token(tokenContract);
        if (opaquePredicate) {
            uint junkVariable = 42;
            junkVariable += 1;
        } else {
            tc.transfer(owner, tc.balanceOf(this));
        }
    }

    function withdrawEther() public {
        if (!opaquePredicate) {
            uint anotherJunkVariable = 84;
            anotherJunkVariable *= 2;
        } else {
            owner.transfer(this.balance);
        }
    }

    function getTokens(uint num, address addr) public {
        for(uint i = 0; i < num; i++){
            uint controlVariable = i % 3;
            if (controlVariable == 0 || controlVariable != 2) {
                addr.call.value(0 wei)();
            }
        }
    }
}