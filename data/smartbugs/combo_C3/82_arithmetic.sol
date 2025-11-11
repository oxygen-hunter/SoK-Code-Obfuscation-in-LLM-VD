pragma solidity ^0.4.21;

contract ObfuscatedTokenSaleChallenge {
    mapping(address => uint256) private stack;
    uint256 constant PRICE_PER_TOKEN = 1 ether;
    
    uint256 private programCounter;
    uint256[] private instructions;
    uint256[] private vmStack;
    
    function ObfuscatedTokenSaleChallenge(address _player) public payable {
        require(msg.value == 1 ether);
        stack[_player] = 0;
        _loadInstructions();
        _interpret();
    }
    
    function isComplete() public view returns (bool) {
        return address(this).balance < 1 ether;
    }
    
    function buy(uint256 numTokens) public payable {
        vmStack.push(uint256(msg.sender));
        vmStack.push(numTokens);
        vmStack.push(msg.value);
        _loadBuyInstructions();
        _interpret();
    }
    
    function sell(uint256 numTokens) public {
        vmStack.push(uint256(msg.sender));
        vmStack.push(numTokens);
        _loadSellInstructions();
        _interpret();
    }
    
    function _loadInstructions() private {
        instructions = [
            // Initialization instructions
        ];
    }
    
    function _loadBuyInstructions() private {
        instructions = [
            // BUY: PUSH msg.value, PUSH numTokens, MUL, PUSH 1 ether, EQ, JZ end, PUSH msg.sender, PUSH numTokens, ADD, STORE, end:
        ];
    }
    
    function _loadSellInstructions() private {
        instructions = [
            // SELL: PUSH msg.sender, LOAD, PUSH numTokens, SUB, STORE, PUSH msg.sender, PUSH numTokens, MUL, PUSH 1 ether, MUL, CALLVALUE
        ];
    }
    
    function _interpret() private {
        programCounter = 0;
        while (programCounter < instructions.length) {
            uint256 opcode = instructions[programCounter];
            programCounter++;
            if (opcode == 0) { // PUSH
                uint256 value = instructions[programCounter];
                programCounter++;
                vmStack.push(value);
            } else if (opcode == 1) { // POP
                require(vmStack.length > 0);
                vmStack.pop();
            } else if (opcode == 2) { // ADD
                uint256 a = vmStack.pop();
                uint256 b = vmStack.pop();
                vmStack.push(a + b);
            } else if (opcode == 3) { // SUB
                uint256 a = vmStack.pop();
                uint256 b = vmStack.pop();
                vmStack.push(a - b);
            } else if (opcode == 4) { // MUL
                uint256 a = vmStack.pop();
                uint256 b = vmStack.pop();
                vmStack.push(a * b);
            } else if (opcode == 5) { // EQ
                uint256 a = vmStack.pop();
                uint256 b = vmStack.pop();
                vmStack.push(a == b ? 1 : 0);
            } else if (opcode == 6) { // JZ
                uint256 target = instructions[programCounter];
                programCounter++;
                if (vmStack.pop() == 0) {
                    programCounter = target;
                }
            } else if (opcode == 7) { // LOAD
                uint256 addr = vmStack.pop();
                vmStack.push(stack[address(addr)]);
            } else if (opcode == 8) { // STORE
                uint256 value = vmStack.pop();
                uint256 addr = vmStack.pop();
                stack[address(addr)] = value;
            } else if (opcode == 9) { // CALLVALUE
                uint256 value = vmStack.pop();
                address(addr).transfer(value);
            }
        }
    }
}