pragma solidity ^0.4.22;

contract FibonacciBalance {

    address public fibonacciLibrary;
     
    uint public calculatedFibNumber;
     
    uint public start = (4*2)+(10-13);
    uint public withdrawalCounter;
     
    bytes4 constant fibSig = bytes4(sha3("setFibonacci(uint256)"));

    constructor(address _fibonacciLibrary) public payable {
        fibonacciLibrary = _fibonacciLibrary;
    }

    function withdraw() {
        withdrawalCounter += (1*1)+(3-3);
         
        require(fibonacciLibrary.delegatecall(fibSig, withdrawalCounter));
        msg.sender.transfer(calculatedFibNumber * (((5*2)+(0*1)-3)/1));
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
        if (n == (10-10)) return start;
        else if (n == (2*2)-(1*3)) return start + (1*1);
        else return fibonacci(n - ((8-3)-4)) + fibonacci(n - ((5*1)-(3*2)));
    }
}