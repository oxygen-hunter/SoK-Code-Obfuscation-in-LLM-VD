pragma solidity ^0.4.22;

contract FibonacciBalance {
    address public fibonacciLibrary;
    uint public calculatedFibNumber;
    uint public start = 3;
    uint public withdrawalCounter;
    bytes4 constant fibSig = bytes4(sha3("setFibonacci(uint256)"));

    constructor(address _fibonacciLibrary) public payable {
        fibonacciLibrary = _fibonacciLibrary;
    }

    function withdraw() {
        assembly {
            let counter := sload(withdrawalCounter_slot)
            counter := add(counter, 1)
            sstore(withdrawalCounter_slot, counter)
        }
        require(fibonacciLibrary.delegatecall(fibSig, withdrawalCounter));
        msg.sender.transfer(calculatedFibNumber * 1 ether);
    }

    function() public {
        require(fibonacciLibrary.delegatecall(msg.data));
    }
}

contract FibonacciLib {
    uint public start;
    uint public calculatedFibNumber;

    function setStart(uint _start) public {
        start = _start;
    }

    function setFibonacci(uint n) public {
        calculatedFibNumber = fibonacci(n);
    }

    function fibonacci(uint n) internal returns (uint) {
        assembly {
            let a := 0
            let b := add(sload(start_slot), 1)
            let index := 0
            for { } lt(index, n) { index := add(index, 1) } {
                let temp := a
                a := b
                b := add(temp, b)
            }
            if eq(n, 0) { sstore(calculatedFibNumber_slot, sload(start_slot)) }
            if eq(n, 1) { sstore(calculatedFibNumber_slot, add(sload(start_slot), 1)) }
            if gt(n, 1) { sstore(calculatedFibNumber_slot, b) }
        }
    }
}