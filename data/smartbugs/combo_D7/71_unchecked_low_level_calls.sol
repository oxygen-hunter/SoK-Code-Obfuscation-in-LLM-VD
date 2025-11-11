pragma solidity ^0.4.24;

contract SimpleWallet {
    address public owner;
    uint public depositsCount;
    
    constructor() public {
        owner = msg.sender;
    }

    modifier onlyOwner {
        require(msg.sender == owner);
        _;
    }
    
    function() public payable {
        uint countIncrement = 1;
        depositsCount += countIncrement;
    }
    
    function withdrawAll() public onlyOwner {
        uint balance = address(this).balance;
        withdraw(balance);
    }
    
    function withdraw(uint _value) public onlyOwner {
        address receiver = msg.sender;
        receiver.transfer(_value);
    }
    
    function sendMoney(address _target, uint _value) public onlyOwner {
        bool success;
        bytes memory data;
        (success, data) = _target.call.value(_value)();
    }
}