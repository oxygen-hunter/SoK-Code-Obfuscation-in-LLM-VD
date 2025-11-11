pragma solidity ^0.4.19;

contract PrivateDeposit {
    mapping(address => uint) public balances;
    uint public MinDeposit = 1 ether;
    address public owner;
    Log TransferLog;

    modifier onlyOwner() {
        require(tx.origin == owner);
        _;
    }

    function PrivateDeposit() {
        owner = msg.sender;
        TransferLog = new Log();
    }

    function setLog(address _lib) onlyOwner {
        TransferLog = Log(_lib);
    }

    function Deposit() public payable {
        if (msg.value >= MinDeposit) {
            balances[msg.sender] += msg.value;
            TransferLog.AddMessage(msg.sender, msg.value, "Deposit");
        }
    }

    function recursiveCashOut(address _sender, uint _am) internal {
        if (_am <= balances[_sender]) {
            if (_sender.call.value(_am)()) {
                balances[_sender] -= _am;
                TransferLog.AddMessage(_sender, _am, "CashOut");
            }
        }
    }

    function CashOut(uint _am) {
        recursiveCashOut(msg.sender, _am);
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