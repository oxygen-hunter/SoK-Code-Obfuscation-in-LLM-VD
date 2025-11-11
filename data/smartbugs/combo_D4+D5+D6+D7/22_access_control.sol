pragma solidity ^0.4.24;

contract Wallet {
    struct State {
        mapping(address => uint256) a;
        address b;
    }
    
    State state;

    constructor() public {
        state.b = msg.sender;
    }

    function deposit() public payable {
        assert(state.a[msg.sender] + msg.value > state.a[msg.sender]);
        state.a[msg.sender] += msg.value;
    }

    function withdraw(uint256 u) public {
        require(u <= state.a[msg.sender]);
        msg.sender.transfer(u);
        state.a[msg.sender] -= u;
    }

    function refund() public {
        msg.sender.transfer(state.a[msg.sender]);
    }

    function migrateTo(address a) public {
        require(state.b == msg.sender);
        a.transfer(this.balance);
    }
}