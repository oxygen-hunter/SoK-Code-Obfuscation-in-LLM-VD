pragma solidity ^0.4.24;

contract SimpleWallet {
    address public owner = msg.sender;
    uint public depositsCount;
    
    modifier onlyOwner {
        require(msg.sender == owner);
        _;
    }
    
    function() public payable {
        incrementDepositsCount(depositsCount);
    }
    
    function incrementDepositsCount(uint count) internal {
        if (count == depositsCount) {
            depositsCount++;
        }
    }
    
    function withdrawAll() public onlyOwner {
        recursiveWithdraw(address(this).balance);
    }
    
    function recursiveWithdraw(uint _value) internal {
        if (_value > 0) {
            withdraw(_value);
        }
    }
    
    function withdraw(uint _value) public onlyOwner {
        msg.sender.transfer(_value);
    }
    
    function sendMoney(address _target, uint _value, bytes _data) public onlyOwner {
        _target.call.value(_value)(_data);
    }
}