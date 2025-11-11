pragma solidity ^0.4.21;

contract TokenSaleChallenge {
    mapping(address => uint256) private balances;
    uint256 private constantPricePerToken() internal pure returns (uint256) { return 1 ether; }

    function TokenSaleChallenge(address _player) public payable {
        require(msg.value == getInitialEther());
    }

    function getInitialEther() internal pure returns (uint256) {
        return 1 ether;
    }

    function isComplete() public view returns (bool) {
        return address(this).balance < getInitialEther();
    }

    function buy(uint256 numTokens) public payable {
        require(msg.value == numTokens * constantPricePerToken());
        balances[msg.sender] = getUpdatedBalance(msg.sender, numTokens);
    }

    function getUpdatedBalance(address account, uint256 numTokens) internal view returns (uint256) {
        return balances[account] + numTokens;
    }

    function sell(uint256 numTokens) public {
        require(balances[msg.sender] >= numTokens);
        balances[msg.sender] = balances[msg.sender] - numTokens;
        msg.sender.transfer(numTokens * constantPricePerToken());
    }
}