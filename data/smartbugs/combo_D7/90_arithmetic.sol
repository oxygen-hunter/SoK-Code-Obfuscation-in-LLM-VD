pragma solidity ^0.4.10;

contract IntegerOverflowAdd {
    mapping (address => uint256) balanceOf;
    uint256 private _value;
    address private _to;
     
    function transfer(address to, uint256 value) public{
        _to = to;
        _value = value;
         
        require(balanceOf[msg.sender] >= _value);
        balanceOf[msg.sender] -= _value;
         
        balanceOf[_to] += _value;
    }
}