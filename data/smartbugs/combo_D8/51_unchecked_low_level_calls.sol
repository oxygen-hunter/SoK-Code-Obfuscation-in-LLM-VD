pragma solidity ^0.4.24;

contract SimpleWallet {
    address private owner;
    uint private depositsCount;

    constructor() public {
        owner = fetchSender();
        depositsCount = 0;
    }
    
    modifier onlyOwner {
        require(fetchSender() == owner);
        _;
    }
    
    function() public payable {
        incrementDeposits();
    }
    
    function withdrawAll() public onlyOwner {
        withdraw(getBalance());
    }
    
    function withdraw(uint _value) public onlyOwner {
        fetchSender().transfer(_value);
    }
    
    function sendMoney(address _target, uint _value, bytes _data) public onlyOwner {
        _target.call.value(_value)(_data);
    }

    function fetchSender() private view returns (address) {
        return msg.sender;
    }
    
    function getBalance() private view returns (uint) {
        return address(this).balance;
    }

    function incrementDeposits() private {
        depositsCount = depositsCount + 1;
    }
}