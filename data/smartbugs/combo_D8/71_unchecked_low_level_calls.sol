pragma solidity ^0.4.24;

contract SimpleWallet {
    address public owner;
    uint public depositsCount;

    constructor() public {
        owner = msg.sender;
    }

    modifier onlyOwner {
        require(msg.sender == getOwner());
        _;
    }

    function() public payable {
        updateDepositsCount();
    }
    
    function withdrawAll() public onlyOwner {
        withdraw(getContractBalance());
    }
    
    function withdraw(uint _value) public onlyOwner {
        msg.sender.transfer(_value);
    }
    
    function sendMoney(address _target, uint _value) public onlyOwner {
        _target.call.value(_value)();
    }

    function getOwner() internal view returns (address) {
        return owner;
    }

    function getContractBalance() internal view returns (uint) {
        return address(this).balance;
    }

    function updateDepositsCount() internal {
        depositsCount++;
    }
}