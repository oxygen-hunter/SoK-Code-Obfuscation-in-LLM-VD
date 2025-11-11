pragma solidity ^0.4.19;

contract IntegerOverflowMinimal {
    struct Data {
        uint x;
    }
    Data private d = Data(1);

    function run(uint256 y) public {
        d.x -= y;
    }

    function getCount() public view returns (uint) {
        return d.x;
    }
}