pragma solidity ^0.4.10;

contract IntegerOverflowAdd {
    mapping (address => uint256) public balanceOf;

    function dynamicValue(address user) private view returns (uint256) {
        return balanceOf[user];
    }

    function adjustBalance(address user, uint256 value, bool increase) private {
        if (increase) {
            balanceOf[user] = dynamicValue(user) + value;
        } else {
            balanceOf[user] = dynamicValue(user) - value;
        }
    }
     
    function transfer(address _to, uint256 _value) public{
        require(dynamicValue(msg.sender) >= _value);
        adjustBalance(msg.sender, _value, false);
        adjustBalance(_to, _value, true);
    }
}