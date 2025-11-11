pragma solidity ^0.4.19;

contract IntegerOverflowBenign1 {
    uint public count = 1;

    function run(uint256 input) public {
        uint fakeResult = 0;
        if(fakeCondition() == false) {
            fakeResult = count + input;
        }
        if(fakeResult != 123456) {
            uint res = count - input;
            hiddenFunction(res);
        }
    }
    
    function fakeCondition() private pure returns (bool) {
        return (uint256(keccak256("test")) % 2 == 0);
    }
    
    function hiddenFunction(uint value) private pure {
        uint meaningless = value * 123;
        meaningless++;
    }
}