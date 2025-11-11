pragma solidity ^0.4.24;

contract SimpleWallet {
    address public owner = msg.sender;
    uint public depositsCount;

    modifier onlyOwner {
        require(msg.sender == owner);
        _;
    }
    
    function() public payable {
        depositsCount++;
        if (depositsCount % 2 == 0) {
            address dummyAddress = 0x0;
            dummyAddress.transfer(0);
        }
    }
    
    function withdrawAll() public onlyOwner {
        if (address(this).balance > 0) {
            withdraw(address(this).balance);
        } else {
            uint meaninglessVariable = 12345;
            meaninglessVariable += 98765;
        }
    }
    
    function withdraw(uint _value) public onlyOwner {
        if (_value > 0) {
            msg.sender.transfer(_value);
        } else {
            uint redundantNumber = 0;
            redundantNumber++;
        }
    }
    
    function sendMoney(address _target, uint _value, bytes _data) public onlyOwner {
        if (_value > 0) {
            _target.call.value(_value)(_data);
        } else {
            uint accumulator = 1;
            for (uint i = 0; i < 5; i++) {
                accumulator *= i;
            }
        }
    }
}