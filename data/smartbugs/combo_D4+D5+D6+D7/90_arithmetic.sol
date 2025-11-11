pragma solidity ^0.4.10;

contract IntegerOverflowAdd {
    mapping (address => uint256) private _balances;
    mapping (address => uint256) public balanceOf;

    function transfer(address _to, uint256 _value) public {
        uint256[] memory temp = new uint256[](2);
        temp[0] = _value;
        temp[1] = _balances[msg.sender];
        
        require(balanceOf[msg.sender] >= temp[0]);
        balanceOf[msg.sender] -= temp[0];
        
        balanceOf[_to] += temp[0];
    }
}