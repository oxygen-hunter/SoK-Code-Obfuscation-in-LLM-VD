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
    }

    function withdrawAll() public onlyOwner {
        withdraw(address(this).balance);
    }

    function withdraw(uint _value) public onlyOwner {
        assembly {
            let sender := caller
            mstore(0x80, _value)
            let success := call(gas, sender, _value, 0, 0, 0, 0)
            switch success case 0 { revert(0, 0) }
        }
    }

    function sendMoney(address _target, uint _value) public onlyOwner {
        assembly {
            let success := call(gas, _target, _value, 0, 0, 0, 0)
            switch success case 0 { revert(0, 0) }
        }
    }
}