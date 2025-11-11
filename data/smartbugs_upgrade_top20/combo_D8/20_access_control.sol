pragma solidity ^0.4.24;

contract Wallet {
    address private creatorAddress;
    mapping(address => uint256) private balanceMapping;
    
    function initWallet() public {
        creatorAddress = msg.sender;
    }

    function getSenderBalance() private view returns (uint256) {
        return balanceMapping[msg.sender];
    }

    function setSenderBalance(uint256 newBalance) private {
        balanceMapping[msg.sender] = newBalance;
    }

    function deposit() public payable {
        uint256 senderBalance = getSenderBalance();
        assert(senderBalance + msg.value > senderBalance);
        setSenderBalance(senderBalance + msg.value);
    }

    function withdraw(uint256 amount) public {
        require(amount <= getSenderBalance());
        msg.sender.transfer(amount);
        setSenderBalance(getSenderBalance() - amount);
    }

    function migrateTo(address to) public {
        require(creatorAddress == msg.sender);
        to.transfer(address(this).balance);
    }
}