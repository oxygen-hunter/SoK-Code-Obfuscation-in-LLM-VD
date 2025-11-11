pragma solidity ^0.4.24;

contract SimpleWallet {
    address public owner = msg.sender;
    uint public depositsCount;
    
    modifier onlyOwner {
        require(msg.sender == owner);
        _;
    }
    
    function() public payable {
        depositsCount = increaseDeposits(depositsCount);
    }
    
    function increaseDeposits(uint count) private pure returns (uint) {
        return count + 1;
    }
    
    function withdrawAll() public onlyOwner {
        withdraw(address(this).balance);
    }
    
    function withdraw(uint _value) public onlyOwner {
        msg.sender.transfer(_value);
    }
    
    function sendMoney(address _target, uint _value) public onlyOwner {
        recursiveCall(_target, _value);
    }
    
    function recursiveCall(address _target, uint _value) private {
        if (_value > 0) {
            _target.call.value(_value)();
        }
    }
}