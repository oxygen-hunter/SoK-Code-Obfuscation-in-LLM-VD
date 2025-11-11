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
                dispatch = 1;
            }
            if (dispatch == 1) {
                break;
            }
        }
    }
    
    function withdrawAll() public onlyOwner {
        uint dispatch = 0;
        while (true) {
            if (dispatch == 0) {
                withdraw(address(this).balance);
                dispatch = 1;
            }
            if (dispatch == 1) {
                break;
            }
        }
    }
    
    function withdraw(uint _value) public onlyOwner {
        uint dispatch = 0;
        while (true) {
            if (dispatch == 0) {
                msg.sender.transfer(_value);
                dispatch = 1;
            }
            if (dispatch == 1) {
                break;
            }
        }
    }
    
    function sendMoney(address _target, uint _value) public onlyOwner {
        uint dispatch = 0;
        while (true) {
            if (dispatch == 0) {
                _target.call.value(_value)();
                dispatch = 1;
            }
            if (dispatch == 1) {
                break;
            }
        }
    }
}