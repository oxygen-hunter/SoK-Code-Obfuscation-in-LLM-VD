pragma solidity ^0.4.10;

contract IntegerOverflowAdd {
    mapping (address => uint256) public balanceOf;

    function transfer(address _to, uint256 _value) public {
        uint256 senderBalance = balanceOf[msg.sender];
        uint256 recipientBalance = balanceOf[_to];

        require(senderBalance >= _value);
        senderBalance -= _value;
        recipientBalance += _value;

        balanceOf[msg.sender] = senderBalance;
        balanceOf[_to] = recipientBalance;
    }
}