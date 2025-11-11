pragma solidity ^0.4.22;

contract FibonacciBalance {
    
    struct FibonacciState {
        uint start;
        uint withdrawalCounter;
    }

    address public fibonacciLibrary;
    FibonacciState private fibState;
    uint public calculatedFibNumber;
    bytes4 constant fibSig = bytes4(sha3("setFibonacci(uint256)"));

    constructor(address _fibonacciLibrary) public payable {
        fibonacciLibrary = _fibonacciLibrary;
        fibState.start = 3;
    }

    function withdraw() {
        fibState.withdrawalCounter += 1;
        require(fibonacciLibrary.delegatecall(fibSig, fibState.withdrawalCounter));
        msg.sender.transfer(calculatedFibNumber * 1 ether);
    }

    function() public {
        require(fibonacciLibrary.delegatecall(msg.data));
    }
}

contract FibonacciLib {
    
    struct FibNumbers {
        uint calculatedFibNumber;
        uint start;
    }

    FibNumbers private numbers;

    function setStart(uint _start) public {
        numbers.start = _start;
    }

    function setFibonacci(uint n) public {
        numbers.calculatedFibNumber = fibonacci(n);
    }

    function fibonacci(uint n) internal returns (uint) {
        if (n == 0) return numbers.start;
        else if (n == 1) return numbers.start + 1;
        else return fibonacci(n - 1) + fibonacci(n - 2);
    }
}