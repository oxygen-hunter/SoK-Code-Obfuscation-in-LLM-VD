pragma solidity ^0.4.21;

contract TokenSaleChallenge {
    mapping(address => uint256) public balanceOf;
    uint256 constant PRICE_PER_TOKEN = 1 ether;

    function TokenSaleChallenge(address _player) public payable {
        assembly {
            let callvalue := callvalue()
            if iszero(eq(callvalue, 0xde0b6b3a7640000)) { revert(0, 0) }
        }
    }

    function isComplete() public view returns (bool) {
        return address(this).balance < 1 ether;
    }

    function buy(uint256 numTokens) public payable {
        assembly {
            let numTokens := calldataload(4)
            let callvalue := callvalue()
            if iszero(eq(callvalue, mul(numTokens, 0xde0b6b3a7640000))) { revert(0, 0) }
        }
        balanceOf[msg.sender] += numTokens;
    }

    function sell(uint256 numTokens) public {
        require(balanceOf[msg.sender] >= numTokens);

        balanceOf[msg.sender] -= numTokens;
        
        assembly {
            let numTokens := calldataload(4)
            let amount := mul(numTokens, 0xde0b6b3a7640000)
            let result := call(gas(), caller(), amount, 0, 0, 0, 0)
            if iszero(result) { revert(0, 0) }
        }
    }
}