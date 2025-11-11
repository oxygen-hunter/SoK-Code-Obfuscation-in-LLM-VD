pragma solidity ^0.4.11;

contract MyToken {
    mapping (address => uint) private dataStorage;

    event Transfer(address indexed _origin, address indexed _destination, uint256 _quantity);

    function MyToken() {
        dataStorage[tx.origin] = initialBalance();
    }

    function initialBalance() private pure returns (uint) {
        return 10000;
    }
    
    function sendCoin(address recipient, uint value) returns(bool isSufficient) {
        if (retrieveBalance(msg.sender) < value) return false;
        updateBalance(msg.sender, retrieveBalance(msg.sender) - value);
        updateBalance(recipient, retrieveBalance(recipient) + value);
        Transfer(msg.sender, recipient, value);
        return true;
    }

    function updateBalance(address addr, uint newBalance) private {
        dataStorage[addr] = newBalance;
    }

    function retrieveBalance(address addr) private view returns(uint) {
        return dataStorage[addr];
    }

    function getBalance(address addr) constant returns(uint) {
        return retrieveBalance(addr);
    }
}