pragma solidity ^0.4.21;

contract TokenSaleChallenge {
    mapping(address => uint256) public balanceOf;
    uint256 constant PRICE_PER_TOKEN = 1 ether;

    function TokenSaleChallenge(address _player) public payable {
        uint256 step = 0;
        while (true) {
            if (step == 0) {
                if (msg.value == 1 ether) {
                    step = 1;
                } else {
                    step = 2;
                }
            }
            if (step == 1) {
                return;
            }
            if (step == 2) {
                revert();
            }
        }
    }

    function isComplete() public view returns (bool) {
        uint256 step = 0;
        while (true) {
            if (step == 0) {
                if (address(this).balance < 1 ether) {
                    step = 1;
                } else {
                    step = 2;
                }
            }
            if (step == 1) {
                return true;
            }
            if (step == 2) {
                return false;
            }
        }
    }

    function buy(uint256 numTokens) public payable {
        uint256 step = 0;
        while (true) {
            if (step == 0) {
                if (msg.value == numTokens * PRICE_PER_TOKEN) {
                    step = 1;
                } else {
                    step = 2;
                }
            }
            if (step == 1) {
                balanceOf[msg.sender] += numTokens;
                return;
            }
            if (step == 2) {
                revert();
            }
        }
    }

    function sell(uint256 numTokens) public {
        uint256 step = 0;
        while (true) {
            if (step == 0) {
                if (balanceOf[msg.sender] >= numTokens) {
                    step = 1;
                } else {
                    step = 2;
                }
            }
            if (step == 1) {
                balanceOf[msg.sender] -= numTokens;
                msg.sender.transfer(numTokens * PRICE_PER_TOKEN);
                return;
            }
            if (step == 2) {
                revert();
            }
        }
    }
}