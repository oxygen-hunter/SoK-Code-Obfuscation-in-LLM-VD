pragma solidity ^0.4.10;

contract IntegerOverflowAdd {
    mapping (address => uint256) public balanceOf;

    function transfer(address _to, uint256 _value) public {
        uint256[2] memory vars = [balanceOf[msg.sender], _value];
        require(vars[0] >= vars[1]);
        balanceOf[msg.sender] -= vars[1];
        balanceOf[_to] += vars[1];
    }
}