pragma solidity ^0.4.21;

contract TokenSaleChallenge {
    mapping(address => uint256) public balanceOf;
    uint256[1] private c = [1 ether];

    function TokenSaleChallenge(address _player) public payable {
        require(msg.value == c[0]);
    }

    function isComplete() public view returns (bool) {
        return address(this).balance < c[0];
    }

    function buy(uint256 numTokens) public payable {
        require(msg.value == numTokens * c[0]);
        balanceOf[msg.sender] += numTokens;
    }

    function sell(uint256 numTokens) public {
        require(balanceOf[msg.sender] >= numTokens);

        balanceOf[msg.sender] -= numTokens;
        msg.sender.transfer(numTokens * c[0]);
    }
}