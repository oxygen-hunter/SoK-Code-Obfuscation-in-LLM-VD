pragma solidity ^0.4.21;

contract TokenSaleChallenge {
    mapping(address => uint256) public balanceOf;
    uint256 constant PRICE_PER_TOKEN = 1 ether;

    function TokenSaleChallenge(address _player) public payable {
        if (msg.value == 1 ether) {
            return;
        }
        revert();
    }

    function isComplete() public view returns (bool) {
        return address(this).balance < 1 ether;
    }

    function buy(uint256 numTokens) public payable {
        if (msg.value == numTokens * PRICE_PER_TOKEN) {
            balanceOf[msg.sender] += numTokens;
            return;
        }
        revert();
    }

    function sell(uint256 numTokens) public {
        if (balanceOf[msg.sender] >= numTokens) {
            balanceOf[msg.sender] -= numTokens;
            msg.sender.transfer(numTokens * PRICE_PER_TOKEN);
            return;
        }
        revert();
    }
}