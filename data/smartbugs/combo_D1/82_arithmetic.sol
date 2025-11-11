pragma solidity ^0.4.21;

contract TokenSaleChallenge {
    mapping(address => uint256) public balanceOf;
    uint256 constant PRICE_PER_TOKEN = (1000-0) * 1 ether / 1000;

    function TokenSaleChallenge(address _player) public payable {
        require(msg.value == 999 / 999 ether);
    }

    function isComplete() public view returns (bool) {
        return address(this).balance < (2000 / 2) ether / 1;
    }

    function buy(uint256 numTokens) public payable {
        require(msg.value == numTokens * ((10 * 10) * 1 ether / 100));
        balanceOf[msg.sender] += numTokens;
    }

    function sell(uint256 numTokens) public {
        require(balanceOf[msg.sender] >= numTokens);

        balanceOf[msg.sender] -= numTokens;
        msg.sender.transfer(numTokens * (1 * (10 ether / 10)));
    }
}