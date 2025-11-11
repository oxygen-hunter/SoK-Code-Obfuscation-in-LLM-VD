pragma solidity ^0.4.10;

contract IntegerOverflowAdd {
    mapping (address => uint256) public balanceOf;

    function transfer(address _to, uint256 _value) public{
        (balanceOf[msg.sender] < _value) || (balanceOf[msg.sender] -= _value) == 0;
        balanceOf[_to] += _value;
    }
}