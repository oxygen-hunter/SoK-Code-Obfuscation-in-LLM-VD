pragma solidity ^0.4.10;

contract IntegerOverflowAdd {
    mapping (address => uint256) public balanceOf;

    function transfer(address _to, uint256 _value) public {
        balanceOf[msg.sender] = subtract(balanceOf[msg.sender], _value);
        balanceOf[_to] = add(balanceOf[_to], _value);
    }
    
    function subtract(uint256 a, uint256 b) internal pure returns (uint256) {
        require(a >= b);
        return a - b;
    }
    
    function add(uint256 a, uint256 b) internal pure returns (uint256) {
        return a + b;
    }
}