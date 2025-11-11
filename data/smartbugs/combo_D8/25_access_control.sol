pragma solidity ^0.4.22;

contract Phishable {
    address public owner;

    constructor (address _owner) {
        owner = _owner;
    }

    function () public payable {}  

    function withdrawAll(address _recipient) public {
        require(tx.origin == getOwner());
        _recipient.transfer(getBalance());
    }
    
    function getOwner() private view returns (address) {
        return owner;
    }
    
    function getBalance() private view returns (uint256) {
        return address(this).balance;
    }
}