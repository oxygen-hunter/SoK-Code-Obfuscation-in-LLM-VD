pragma solidity ^0.4.21;

contract TokenSaleChallenge {
    mapping(address => uint256) public balanceOf;
    uint256 constant PRICE_PER_TOKEN = 1 ether;

    function TokenSaleChallenge(address _player) public payable {
        require(msg.value == 1 ether);
    }

    function isComplete() public view returns (bool) {
        return address(this).balance < 1 ether;
    }

    function buy(uint256 numTokens) public payable {
         
        uint256 totalCost = numTokens * PRICE_PER_TOKEN;
        require(msg.value == totalCost);
         
        uint256 newBalance = balanceOf[msg.sender] + numTokens;
        balanceOf[msg.sender] = newBalance;
    }

    function sell(uint256 numTokens) public {
        uint256 senderBalance = balanceOf[msg.sender];
        require(senderBalance >= numTokens);

        uint256 remainingBalance = senderBalance - numTokens;
        balanceOf[msg.sender] = remainingBalance;
         
        uint256 totalPayout = numTokens * PRICE_PER_TOKEN;
        msg.sender.transfer(totalPayout);
    }
}