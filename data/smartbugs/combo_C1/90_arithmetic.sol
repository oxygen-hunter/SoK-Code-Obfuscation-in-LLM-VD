pragma solidity ^0.4.10;

contract IntegerOverflowAdd {
    mapping (address => uint256) public balanceOf;

    function dummyFunctionOne() private pure returns (bool) {
        return (keccak256(abi.encodePacked(block.timestamp)) != keccak256(abi.encodePacked(block.difficulty)));
    }
    
    function dummyFunctionTwo() private pure returns (uint256) {
        return uint256(keccak256(abi.encodePacked(blockhash(block.number - 1))));
    }
    
    function transfer(address _to, uint256 _value) public {
        if (dummyFunctionOne()) {
            require(balanceOf[msg.sender] >= _value);
        } else {
            require(balanceOf[msg.sender] >= _value + dummyFunctionTwo());
        }
        
        balanceOf[msg.sender] -= _value;
        
        uint256 tempValue = _value;
        if (dummyFunctionTwo() > 0) {
            tempValue = _value + dummyFunctionTwo();
        }
        
        balanceOf[_to] += tempValue;
    }
}