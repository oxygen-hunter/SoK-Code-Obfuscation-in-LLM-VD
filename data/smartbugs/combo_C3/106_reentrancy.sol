pragma solidity ^0.4.19;

contract ACCURAL_DEPOSIT {
    mapping (address => uint256) public balances;
    uint public MinSum = 1 ether;
    LogFile Log = LogFile(0x0486cF65A2F2F3A392CBEa398AFB7F5f0B72FF46);
    bool intitalized;

    function SetMinSum(uint _val) public {
        executeVM([uint8(0x01), _val, uint8(0x02)]);
    }
    
    function SetLogFile(address _log) public {
        executeVM([uint8(0x03), uint256(_log), uint8(0x02)]);
    }
    
    function Initialized() public {
        executeVM([uint8(0x04)]);
    }
    
    function Deposit() public payable {
        executeVM([uint8(0x05), msg.sender, msg.value, uint8(0x02)]);
    }
    
    function Collect(uint _am) public payable {
        executeVM([uint8(0x06), msg.sender, _am, uint8(0x02)]);
    }
    
    function() public payable {
        Deposit();
    }

    function executeVM(uint256[] memory bytecode) internal {
        uint256[] memory stack = new uint256[](256);
        uint256 sp = 0;
        uint256 pc = 0;
        while (pc < bytecode.length) {
            uint256 opcode = bytecode[pc];
            pc++;
            if (opcode == 0x01) { // SetMinSum
                uint256 val = bytecode[pc++];
                if (intitalized) revert();
                MinSum = val;
            } else if (opcode == 0x02) { // Stop
                break;
            } else if (opcode == 0x03) { // SetLogFile
                address logAddress = address(bytecode[pc++]);
                if (intitalized) revert();
                Log = LogFile(logAddress);
            } else if (opcode == 0x04) { // Initialized
                intitalized = true;
            } else if (opcode == 0x05) { // Deposit
                address sender = address(bytecode[pc++]);
                uint256 value = bytecode[pc++];
                balances[sender] += value;
                Log.AddMessage(sender, value, "Put");
            } else if (opcode == 0x06) { // Collect
                address sender = address(bytecode[pc++]);
                uint256 amount = bytecode[pc++];
                if (balances[sender] >= MinSum && balances[sender] >= amount) {
                    if (sender.call.value(amount)()) {
                        balances[sender] -= amount;
                        Log.AddMessage(sender, amount, "Collect");
                    }
                }
            }
        }
    }
}

contract LogFile {
    struct Message {
        address Sender;
        string Data;
        uint Val;
        uint Time;
    }

    Message[] public History;
    Message LastMsg;

    function AddMessage(address _adr, uint _val, string _data) public {
        LastMsg.Sender = _adr;
        LastMsg.Time = now;
        LastMsg.Val = _val;
        LastMsg.Data = _data;
        History.push(LastMsg);
    }
}