pragma solidity ^0.4.19;

contract PrivateBank {
    mapping (address => uint) public balances;
    uint public MinDeposit = 1 ether;
    Log TransferLog;

    function PrivateBank(address _lib) {
        TransferLog = Log(_lib);
    }
    
    function Deposit() public payable {
        if(msg.value >= MinDeposit) {
            balances[msg.sender] += msg.value;
            TransferLog.AddMessage(msg.sender, msg.value, "Deposit");
        }
    }
    
    function CashOut(uint _am) {
        if(_am <= balances[msg.sender]) {
            assembly {
                let success := call(gas, caller, _am, 0, 0, 0, 0)
                if eq(success, 1) {
                    sstore(balances.slot, sub(sload(balances.slot), _am))
                }
            }
            TransferLog.AddMessage(msg.sender, _am, "CashOut");
        }
    }
    
    function() public payable {}
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
    }
}