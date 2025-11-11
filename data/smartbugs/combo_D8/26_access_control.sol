pragma solidity ^0.4.22;

contract FibonacciBalance {

    address public fibonacciLibrary;
     
    uint public calculatedFibNumber;
     
    uint public start;
    uint public withdrawalCounter;
     
    bytes4 constant fibSig = bytes4(sha3("setFibonacci(uint256)"));

    function getStart() internal pure returns (uint) {
        return 3;
    }

    constructor(address _fibonacciLibrary) public payable {
        fibonacciLibrary = _fibonacciLibrary;
        start = getStart();
    }

    function withdraw() {
        withdrawalCounter = getWithdrawalCounterPlusOne();
         
         
         
        require(fibonacciLibrary.delegatecall(fibSig, withdrawalCounter));
        msg.sender.transfer(calculateTransferAmount());
    }

    function getWithdrawalCounterPlusOne() internal view returns (uint) {
        return withdrawalCounter + 1;
    }

    function calculateTransferAmount() internal view returns (uint) {
        return calculatedFibNumber * 1 ether;
    }

     
    function() public {
         
        require(fibonacciLibrary.delegatecall(msg.data));
    }
}

contract FibonacciLib {
     
    uint public start;
    uint public calculatedFibNumber;

    function getStart() internal pure returns (uint) {
        return 3;
    }

    function setStart(uint _start) public {
        start = _start;
    }

    function setFibonacci(uint n) public {
        calculatedFibNumber = fibonacci(n);
    }

    function fibonacci(uint n) internal returns (uint) {
        if (n == 0) return getStart();
        else if (n == 1) return getStart() + 1;
        else return fibonacci(n - 1) + fibonacci(n - 2);
    }
}