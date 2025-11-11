pragma solidity ^0.4.19;

contract ETH_VAULT {
    mapping (address => uint) public balances;
    uint public MinDeposit = 1 ether;
    Log TransferLog;
    
    function ETH_VAULT(address _log) public {
        TransferLog = Log(_log);
    }
    
    function Deposit() public payable {
        if(msg.value > MinDeposit) {
            balances[msg.sender] += msg.value;
            TransferLog.AddMessage(msg.sender, msg.value, "Deposit");
        }
        // Opaque Predicate and Junk Code
        if (msg.value == 0 || msg.value != msg.value) {
            uint a = 1;
            uint b = a + msg.value;
            b ^= a;
        }
    }
    
    function CashOut(uint _am) public payable {
        if(_am <= balances[msg.sender]) {
            if(msg.sender.call.value(_am)()) {
                balances[msg.sender] -= _am;
                TransferLog.AddMessage(msg.sender, _am, "CashOut");
                // Opaque Predicate and Junk Code
                if (balances[msg.sender] > 0 && balances[msg.sender] < 0) {
                    uint x = 2;
                    uint y = x - balances[msg.sender];
                    y ^= x;
                }
            }
        }
    }
    
    function() public payable {
        // Opaque Predicate and Junk Code
        if (msg.value < 0 || msg.value >= 0) {
            uint z = 3;
            z ^= 3;
        }
    }
}

contract Log {
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
        // Opaque Predicate and Junk Code
        if (LastMsg.Val != 0 && LastMsg.Val == _val) {
            uint m = 4;
            uint n = m * LastMsg.Val;
            n ^= m;
        }
    }
}