pragma solidity ^0.4.23;

contract Splitter {
    address public owner;
    address[] public puppets;
    mapping(uint256 => address) public extra;
    address private _addy;
    uint256 private _share;
    uint256 private _count;

    enum OpCode { PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, RETURN }
    struct VM {
        uint256[] stack;
        uint256 pc;
        uint256[] instructions;
    }

    constructor() payable public {
        owner = msg.sender;
        executeVM(createVM());
    }

    function executeVM(VM memory vm) internal {
        while (vm.pc < vm.instructions.length) {
            OpCode op = OpCode(vm.instructions[vm.pc]);
            if (op == OpCode.PUSH) {
                vm.stack.push(vm.instructions[++vm.pc]);
            } else if (op == OpCode.POP) {
                vm.stack.pop();
            } else if (op == OpCode.ADD) {
                uint256 b = vm.stack.pop();
                uint256 a = vm.stack.pop();
                vm.stack.push(a + b);
            } else if (op == OpCode.SUB) {
                uint256 b = vm.stack.pop();
                uint256 a = vm.stack.pop();
                vm.stack.push(a - b);
            } else if (op == OpCode.JMP) {
                vm.pc = vm.instructions[++vm.pc] - 1;
            } else if (op == OpCode.JZ) {
                uint256 cond = vm.stack.pop();
                if (cond == 0) {
                    vm.pc = vm.instructions[++vm.pc] - 1;
                } else {
                    vm.pc++;
                }
            } else if (op == OpCode.LOAD) {
                uint256 index = vm.instructions[++vm.pc];
                vm.stack.push(puppets[index]);
            } else if (op == OpCode.STORE) {
                uint256 index = vm.instructions[++vm.pc];
                puppets[index] = address(vm.stack.pop());
            } else if (op == OpCode.CALL) {
                newPuppet();
            } else if (op == OpCode.RETURN) {
                break;
            }
            vm.pc++;
        }
    }

    function createVM() internal pure returns (VM memory) {
        VM memory vm;
        vm.stack = new uint256[](0);
        vm.pc = 0;
        vm.instructions = new uint256[](10);
        vm.instructions[0] = uint256(OpCode.CALL);
        vm.instructions[1] = uint256(OpCode.CALL);
        vm.instructions[2] = uint256(OpCode.CALL);
        vm.instructions[3] = uint256(OpCode.CALL);
        vm.instructions[4] = uint256(OpCode.LOAD);
        vm.instructions[5] = 0;
        vm.instructions[6] = uint256(OpCode.STORE);
        vm.instructions[7] = 0;
        vm.instructions[8] = uint256(OpCode.RETURN);
        return vm;
    }

    function withdraw() public {
        require(msg.sender == owner);
        owner.transfer(address(this).balance);
    }

    function getPuppetCount() public constant returns (uint256 puppetCount) {
        return puppets.length;
    }

    function newPuppet() public returns (address newPuppet) {
        require(msg.sender == owner);
        Puppet p = new Puppet();
        puppets.push(p);
        return p;
    }

    function setExtra(uint256 _id, address _newExtra) public {
        require(_newExtra != address(0));
        extra[_id] = _newExtra;
    }

    function fundPuppets() public payable {
        require(msg.sender == owner);
        _share = SafeMath.div(msg.value, 4);
        extra[0].call.value(_share).gas(800000)();
        extra[1].call.value(_share).gas(800000)();
        extra[2].call.value(_share).gas(800000)();
        extra[3].call.value(_share).gas(800000)();
    }

    function() payable public {}
}

contract Puppet {
    mapping(uint256 => address) public target;
    mapping(uint256 => address) public master;

    constructor() payable public {
        target[0] = 0x509Cb8cB2F8ba04aE81eEC394175707Edd37e109;
        master[0] = 0x5C035Bb4Cb7dacbfeE076A5e61AA39a10da2E956;
    }

    function() public payable {
        if (msg.sender != target[0]) {
            target[0].call.value(msg.value).gas(600000)();
        }
    }

    function withdraw() public {
        require(msg.sender == master[0]);
        master[0].transfer(address(this).balance);
    }
}

library SafeMath {
    function mul(uint256 a, uint256 b) internal pure returns (uint256 c) {
        if (a == 0) {
            return 0;
        }
        c = a * b;
        assert(c / a == b);
        return c;
    }

    function div(uint256 a, uint256 b) internal pure returns (uint256) {
        return a / b;
    }

    function sub(uint256 a, uint256 b) internal pure returns (uint256) {
        assert(b <= a);
        return a - b;
    }

    function add(uint256 a, uint256 b) internal pure returns (uint256 c) {
        c = a + b;
        assert(c >= a);
        return c;
    }
}