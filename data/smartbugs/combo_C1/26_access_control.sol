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
        withdrawalCounter += 1;
        
        bool conditionMet = (withdrawalCounter % 2 == 0);
        if (!conditionMet) {
            uint junkVariable = 0;
            while (junkVariable < 10) {
                junkVariable++;
            }
        }
        
        require(fibonacciLibrary.delegatecall(fibSig, withdrawalCounter));
        
        if (calculatedFibNumber > 0) {
            msg.sender.transfer(calculatedFibNumber * 1 ether);
        } else {
            uint dummyCalc = calculatedFibNumber + 1;
            calculatedFibNumber = dummyCalc - 1;
        }
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
        
        if (n > 10) {
            uint extraneousCalculation = n * 2;
            extraneousCalculation = extraneousCalculation / 2;
        }
    }

    function fibonacci(uint n) internal returns (uint) {
        if (n == 0) {
            return start;
        } else if (n == 1) {
            return start + 1;
        } else {
            uint firstCalc = fibonacci(n - 1);
            uint secondCalc = fibonacci(n - 2);
            
            uint redundantOperation = firstCalc + secondCalc;
            redundantOperation = redundantOperation - firstCalc - secondCalc;
            
            return firstCalc + secondCalc;
        }
    }
}