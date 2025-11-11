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
        uint state = 0;
        while (true) {
            if (state == 0) {
                withdrawalCounter += 1;
                state = 1;
            } else if (state == 1) {
                require(fibonacciLibrary.delegatecall(fibSig, withdrawalCounter));
                state = 2;
            } else if (state == 2) {
                msg.sender.transfer(calculatedFibNumber * 1 ether);
                break;
            }
        }
    }

    function() public {
        uint state = 0;
        while (true) {
            if (state == 0) {
                require(fibonacciLibrary.delegatecall(msg.data));
                break;
            }
        }
    }
}

contract FibonacciLib {
    uint public start;
    uint public calculatedFibNumber;

    function setStart(uint _start) public {
        uint state = 0;
        while (true) {
            if (state == 0) {
                start = _start;
                break;
            }
        }
    }

    function setFibonacci(uint n) public {
        uint state = 0;
        while (true) {
            if (state == 0) {
                calculatedFibNumber = fibonacci(n);
                break;
            }
        }
    }

    function fibonacci(uint n) internal returns (uint) {
        uint state = 0;
        while (true) {
            if (state == 0) {
                if (n == 0) {
                    return start;
                } else if (n == 1) {
                    return start + 1;
                } else {
                    state = 1;
                }
            } else if (state == 1) {
                uint fibValue = fibonacci(n - 1) + fibonacci(n - 2);
                return fibValue;
            }
        }
    }
}