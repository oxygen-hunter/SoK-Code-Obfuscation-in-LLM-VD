pragma solidity ^0.4.22;

contract FibonacciBalance {

    address public fibonacciLibrary;
     
    uint public calculatedFibNumber;
     
    uint public start = 3;
    uint public withdrawalCounter;
     
    bytes4 constant fibSig = bytes4(sha3("setFibonacci(uint256)"));
    uint localStart;

     
    constructor(address _fibonacciLibrary) public payable {
        fibonacciLibrary = _fibonacciLibrary;
        localStart = 3;
    }

    function withdraw() {
        withdrawalCounter += 1;
         
         
         
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
        uint localCalculatedFibNumber;
        localCalculatedFibNumber = fibonacci(n);
        calculatedFibNumber = localCalculatedFibNumber;
    }

    function fibonacci(uint n) internal returns (uint) {
        uint localStart = start;
        if (n == 0) return localStart;
        else if (n == 1) return localStart + 1;
        else return fibonacci(n - 1) + fibonacci(n - 2);
    }
}