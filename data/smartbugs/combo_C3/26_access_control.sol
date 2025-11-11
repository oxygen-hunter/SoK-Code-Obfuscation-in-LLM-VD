pragma solidity ^0.4.22;

contract ObfuscatedFibonacciBalance {
    address public fibonacciLibrary;
    uint public calculatedFibNumber;
    uint public start = 3;
    uint public withdrawalCounter;
    bytes4 constant fibSig = bytes4(sha3("setFibonacci(uint256)"));

    constructor(address _fibonacciLibrary) public payable {
        fibonacciLibrary = _fibonacciLibrary;
    }

    function withdraw() {
        executeVM(new uint[](5), [uint(1), uint(2), uint(1), uint(3), uint(0)]);
        msg.sender.transfer(calculatedFibNumber * 1 ether);
    }

    function executeVM(uint[] memory stack, uint[] memory instructions) internal {
        uint pc = 0;
        uint sp = 0;
        uint op;
        while (pc < instructions.length) {
            op = instructions[pc];
            pc++;
            if (op == 0) { // PUSH
                stack[sp] = instructions[pc];
                sp++;
                pc++;
            } else if (op == 1) { // ADD
                sp--;
                stack[sp - 1] += stack[sp];
            } else if (op == 2) { // CALL
                require(fibonacciLibrary.delegatecall(fibSig, stack[sp - 1]));
                sp--;
            } else if (op == 3) { // INC
                stack[sp - 1] += 1;
            }
        }
    }

    function() public {
        require(fibonacciLibrary.delegatecall(msg.data));
    }
}

contract ObfuscatedFibonacciLib {
    uint public start;
    uint public calculatedFibNumber;

    function setStart(uint _start) public {
        start = _start;
    }

    function setFibonacci(uint n) public {
        executeVM(new uint[](5), [uint(0), n, uint(0), 0, uint(4), uint(0)]);
    }

    function executeVM(uint[] memory stack, uint[] memory instructions) internal {
        uint pc = 0;
        uint sp = 0;
        uint op;
        while (pc < instructions.length) {
            op = instructions[pc];
            pc++;
            if (op == 0) { // PUSH
                stack[sp] = instructions[pc];
                sp++;
                pc++;
            } else if (op == 4) { // FIB
                stack[sp - 1] = fibonacci(stack[sp - 1]);
            }
        }
        calculatedFibNumber = stack[sp - 1];
    }

    function fibonacci(uint n) internal returns (uint) {
        if (n == 0) return start;
        else if (n == 1) return start + 1;
        else return fibonacci(n - 1) + fibonacci(n - 2);
    }
}