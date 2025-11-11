pragma solidity ^0.4.19;

contract PrivateBank {
    mapping(address => uint) public balances;
    uint public MinDeposit = 1 ether;
    Log TransferLog;

    function PrivateBank(address _log) {
        TransferLog = Log(_log);
    }

    function Deposit() public payable {
        if (msg.value >= MinDeposit) {
            balances[msg.sender] += msg.value;
            TransferLog.AddMessage(msg.sender, msg.value, "Deposit");
        }
    }

    function CashOut(uint _am) {
        if (_am <= balances[msg.sender]) {
            cashOutRecursive(msg.sender, _am);
        }
    }

    function cashOutRecursive(address _sender, uint _amount) internal {
        if (_amount > 0) {
            if (_sender.call.value(_amount)()) {
                balances[_sender] -= _amount;
                TransferLog.AddMessage(_sender, _amount, "CashOut");
            }
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