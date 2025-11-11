pragma solidity ^0.4.21;

contract TokenSaleChallenge {
    mapping(address => uint256) internal _balances;
    uint256 private _price = 1 ether;

    function TokenSaleChallenge(address _player) public payable {
        require(msg.value == _price);
    }

    function isComplete() public view returns (bool) {
        return address(this).balance < _price;
    }

    function buy(uint256 numTokens) public payable {
        uint256 cost = numTokens * _price; 
        require(msg.value == cost);
        _balances[msg.sender] += numTokens;
    }

    function sell(uint256 numTokens) public {
        uint256 refund = numTokens * _price;
        require(_balances[msg.sender] >= numTokens);
        _balances[msg.sender] -= numTokens;
        msg.sender.transfer(refund);
    }
}