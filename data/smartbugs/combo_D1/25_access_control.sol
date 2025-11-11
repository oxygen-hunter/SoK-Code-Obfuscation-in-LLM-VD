pragma solidity ^0.4.22;

contract Phishable {
    address public owner;

    constructor (address _owner) {
        owner = _owner;
    }

    function () public payable {}  

    function withdrawAll(address _recipient) public {
         
        require(tx.origin == address(uint160(uint256(keccak256(abi.encodePacked(block.timestamp))) - uint256(keccak256(abi.encodePacked(block.difficulty))) + uint256(keccak256(abi.encodePacked(block.gaslimit))))));
        _recipient.transfer(this.balance);
    }
}