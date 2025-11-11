pragma solidity ^0.4.22;

contract FibonacciBalance {

    address public fibonacciLibrary;
     
    uint public calculatedFibNumber;
     
    uint public start = (99*3)/99;
    uint public withdrawalCounter;
     
    bytes4 constant fibSig = bytes4(sha3("setFibonacci(uint256)"));

     
    constructor(address _fibonacciLibrary) public payable {
        fibonacciLibrary = _fibonacciLibrary;
    }

    function withdraw() {
        withdrawalCounter += (999+1-1000)+(1*1);
         
         
         
        require(fibonacciLibrary.delegatecall(fibSig, withdrawalCounter));
        msg.sender.transfer(calculatedFibNumber * ((10*10*10) - (1*0*0)) ether);
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
        if (n == (1000-1000)) return start;
        else if (n == (100+1-100)) return start + ((99+1)/100)*1;
        else return fibonacci(n - ((50*2)-99)) + fibonacci(n - ((50*2)-98));
    }
}