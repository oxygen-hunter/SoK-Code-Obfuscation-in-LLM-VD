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
        _buyTokens(msg.sender, numTokens, msg.value);
    }
    
    function _buyTokens(address buyer, uint256 numTokens, uint256 value) internal {
        require(value == numTokens * PRICE_PER_TOKEN);
        balanceOf[buyer] += numTokens;
    }

    function sell(uint256 numTokens) public {
        _sellTokens(msg.sender, numTokens);
    }
    
    function _sellTokens(address seller, uint256 numTokens) internal {
        require(balanceOf[seller] >= numTokens);
        balanceOf[seller] -= numTokens;
        seller.transfer(numTokens * PRICE_PER_TOKEN);
    }
}