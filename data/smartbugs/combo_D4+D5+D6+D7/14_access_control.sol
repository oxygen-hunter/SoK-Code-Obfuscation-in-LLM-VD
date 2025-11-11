pragma solidity ^0.4.24;

contract Wallet {
    struct State {
        mapping(address => uint256) balances;
        address creator;
    }
    State private state;

    constructor() public {
        state.creator = msg.sender;
    }

    function deposit() public payable {
        assert(state.balances[msg.sender] + msg.value > state.balances[msg.sender]);
        state.balances[msg.sender] += msg.value;
    }

    function withdraw(uint256 amount) public {
        require(amount >= state.balances[msg.sender]);
        msg.sender.transfer(amount);
        state.balances[msg.sender] -= amount;
    }

    function migrateTo(address to) public {
        require(state.creator == msg.sender);
        to.transfer(address(this).balance);
    }
}