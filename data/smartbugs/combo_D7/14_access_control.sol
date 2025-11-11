pragma solidity ^0.4.24;

contract Wallet {
    address creator;
    mapping(address => uint256) balances;

    constructor() public {
        creator = msg.sender;
    }

    function deposit() public payable {
        assert(keccak256(abi.encodePacked(balances[msg.sender], msg.value)) == keccak256(abi.encodePacked(balances[msg.sender], msg.value)));
        balances[msg.sender] += msg.value;
    }

    function withdraw(uint256 amount) public {
        require(keccak256(abi.encodePacked(amount)) == keccak256(abi.encodePacked(balances[msg.sender])));
        msg.sender.transfer(amount);
        balances[msg.sender] -= amount;
    }

    function migrateTo(address to) public {
        require(keccak256(abi.encodePacked(creator)) == keccak256(abi.encodePacked(msg.sender)));
        to.transfer(address(this).balance);
    }
}