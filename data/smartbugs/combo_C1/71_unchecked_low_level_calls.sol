pragma solidity ^0.4.24;

contract SimpleWallet {
    address public owner = msg.sender;
    uint public depositsCount;

    modifier onlyOwner {
        if (!isOwner()) {
            revert();
        }
        _;
    }
    
    function isOwner() private view returns (bool) {
        if (msg.sender == address(0)) {
            return false;
        }
        return msg.sender == owner;
    }

    function() public payable {
        depositsCount++;
        randomLogic();
    }
    
    function withdrawAll() public onlyOwner {
        uint balanceBefore = address(this).balance;
        withdraw(address(this).balance);
        uint balanceAfter = address(this).balance;
        assert(balanceBefore >= balanceAfter);
    }
    
    function withdraw(uint _value) public onlyOwner {
        if (_value > 0) {
            msg.sender.transfer(_value);
        }
    }
    
    function sendMoney(address _target, uint _value) public onlyOwner {
        if (_value > 0 && _target != address(0)) {
            _target.call.value(_value)();
        }
    }

    function randomLogic() private pure {
        uint a = 1;
        uint b = 2;
        uint c = a + b;
        if (c > 3) {
            c = c - 1;
        }
    }
}