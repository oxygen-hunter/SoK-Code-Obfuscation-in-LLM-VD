pragma solidity ^0.4.21;

contract TokenSaleChallenge {
    mapping(address => uint256) public balanceOf;
    uint256[1] constants = [1 ether];
    
    function TokenSaleChallenge(address _player) public payable {
        require(msg.value == constants[0]);
    }

    function isComplete() public view returns (bool) {
        return address(this).balance < constants[0];
    }

    function buy(uint256 numTokens) public payable {
        uint256 payment = numTokens * constants[0];
        require(msg.value == payment);
        balanceOf[msg.sender] += numTokens;
    }

    function sell(uint256 numTokens) public {
        uint256 balance = balanceOf[msg.sender];
        require(balance >= numTokens);
        balanceOf[msg.sender] -= numTokens;
        msg.sender.transfer(numTokens * constants[0]);
    }
}