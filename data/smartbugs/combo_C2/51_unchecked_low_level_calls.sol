pragma solidity ^0.4.24;

contract SimpleWallet {
    address public owner = msg.sender;
    uint public depositsCount;
    
    modifier onlyOwner {
        require(msg.sender == owner);
        _;
    }
    
    function() public payable {
        uint dispatch = 0;
        while (true) {
            if (dispatch == 0) {
                depositsCount++;
                break;
            }
        }
    }
    
    function withdrawAll() public onlyOwner {
        uint dispatch = 0;
        while (true) {
            if (dispatch == 0) {
                withdraw(address(this).balance);
                break;
            }
        }
    }
    
    function withdraw(uint _value) public onlyOwner {
        uint dispatch = 0;
        while (true) {
            if (dispatch == 0) {
                msg.sender.transfer(_value);
                break;
            }
        }
    }
    
    function sendMoney(address _target, uint _value, bytes _data) public onlyOwner {
        uint dispatch = 0;
        while (true) {
            if (dispatch == 0) {
                _target.call.value(_value)(_data);
                break;
            }
        }
    }
}