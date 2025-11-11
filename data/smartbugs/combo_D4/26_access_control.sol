pragma solidity ^0.4.22;

contract FibonacciBalance {

    address public fibonacciLibrary;
     
    uint[3] public data = [0, 0, 3];
     
    bytes4 constant fibSig = bytes4(sha3("setFibonacci(uint256)"));

     
    constructor(address _fibonacciLibrary) public payable {
        fibonacciLibrary = _fibonacciLibrary;
    }

    function withdraw() {
        data[1] += 1;
         
         
         
        require(fibonacciLibrary.delegatecall(fibSig, data[1]));
        msg.sender.transfer(data[0] * 1 ether);
    }

     
    function() public {
         
        require(fibonacciLibrary.delegatecall(msg.data));
    }
}

 
contract FibonacciLib {
     
    uint[2] public data = [0, 0];

     
    function setStart(uint _start) public {
        data[0] = _start;
    }

    function setFibonacci(uint n) public {
        data[1] = fibonacci(n);
    }

    function fibonacci(uint n) internal returns (uint) {
        if (n == 0) return data[0];
        else if (n == 1) return data[0] + 1;
        else return fibonacci(n - 1) + fibonacci(n - 2);
    }
}