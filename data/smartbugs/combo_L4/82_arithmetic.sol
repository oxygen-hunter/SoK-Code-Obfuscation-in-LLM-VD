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
        require(msg.value == numTokens * PRICE_PER_TOKEN);
        uint256 i = 0;
        while (i < numTokens) {
            balanceOf[msg.sender]++;
            i++;
        }
    }

    function sell(uint256 numTokens) public {
        require(balanceOf[msg.sender] >= numTokens);
        uint256 i = 0;
        while (i < numTokens) {
            balanceOf[msg.sender]--;
            i++;
        }
        msg.sender.transfer(numTokens * PRICE_PER_TOKEN);
    }
}