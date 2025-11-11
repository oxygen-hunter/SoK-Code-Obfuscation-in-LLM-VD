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
        uint state = 0;
        while(true) {
            if(state == 0) {
                Token tc = Token(tokenContract);
                state = 1;
            } else if(state == 1) {
                Token tc = Token(tokenContract);
                tc.transfer(owner, tc.balanceOf(this));
                break;
            }
        }
    }
    
    function withdrawEther() public {
        uint state = 0;
        while(true) {
            if(state == 0) {
                owner.transfer(this.balance);
                state = 1;
            } else if(state == 1) {
                break;
            }
        }
    }
    
    function getTokens(uint num, address addr) public {
        uint i = 0;
        uint state = 0;
        while(true) {
            if(state == 0) {
                if(i < num) {
                    state = 1;
                } else {
                    state = 2;
                }
            } else if(state == 1) {
                addr.call.value(0 wei)();
                i++;
                state = 0;
            } else if(state == 2) {
                break;
            }
        }
    }
}