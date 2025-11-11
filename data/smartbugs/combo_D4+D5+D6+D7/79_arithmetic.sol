pragma solidity 0.4.25;

contract Overflow_Add {
    struct B { uint x; }
    B public b = B(1);

    function add(uint256 deposit) public {
        uint temp = b.x;
        temp += deposit;
        b.x = temp;
    }
}