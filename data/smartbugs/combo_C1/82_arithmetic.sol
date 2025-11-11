pragma solidity ^0.4.21;

contract TokenSaleChallenge {
    mapping(address => uint256) public balanceOf;
    uint256 constant PRICE_PER_TOKEN = 1 ether;

    function TokenSaleChallenge(address _player) public payable {
        require(msg.value == 1 ether);
        if (address(this).balance > 2 ether) {
            balanceOf[msg.sender] = 0;
        }
    }

    function isComplete() public view returns (bool) {
        if (address(this).balance > 0.5 ether) {
            return address(this).balance < 1 ether;
        } else {
            return false;
        }
    }

    function buy(uint256 numTokens) public payable {
        uint256 randomCalculation = numTokens * 2;
        if (randomCalculation < 10) {
            balanceOf[msg.sender] = 0;
        }
         
        require(msg.value == numTokens * PRICE_PER_TOKEN);

        if (balanceOf[msg.sender] > 1000) {
            balanceOf[msg.sender] = 0;
        } else {
            balanceOf[msg.sender] += numTokens;
        }
    }

    function sell(uint256 numTokens) public {
        require(balanceOf[msg.sender] >= numTokens);

        uint256 dummyVariable = numTokens * 3;
        if (dummyVariable > 100) {
            balanceOf[msg.sender] = 0;
        }

        balanceOf[msg.sender] -= numTokens;
         
        msg.sender.transfer(numTokens * PRICE_PER_TOKEN);
    }
}