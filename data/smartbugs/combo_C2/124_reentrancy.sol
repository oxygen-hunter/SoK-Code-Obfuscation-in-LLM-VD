pragma solidity ^0.4.19;

contract PrivateBank
{
    mapping (address => uint) public balances;
    
    uint public MinDeposit = 1 ether;
    
    Log TransferLog;

    function PrivateBank(address _log)
    {
        TransferLog = Log(_log);
    }
    
    function Deposit()
    public
    payable
    {
        uint dispatcher = 0;
        while (true) {
            if (dispatcher == 0) {
                if(msg.value < MinDeposit) {
                    break;
                }
                dispatcher = 1;
            } else if (dispatcher == 1) {
                balances[msg.sender] += msg.value;
                dispatcher = 2;
            } else if (dispatcher == 2) {
                TransferLog.AddMessage(msg.sender, msg.value, "Deposit");
                break;
            }
        }
    }
    
    function CashOut(uint _am)
    {
        uint dispatcher = 0;
        while (true) {
            if (dispatcher == 0) {
                if(_am > balances[msg.sender]) {
                    break;
                }
                dispatcher = 1;
            } else if (dispatcher == 1) {
                if(msg.sender.call.value(_am)()) {
                    dispatcher = 2;
                } else {
                    break;
                }
            } else if (dispatcher == 2) {
                balances[msg.sender] -= _am;
                dispatcher = 3;
            } else if (dispatcher == 3) {
                TransferLog.AddMessage(msg.sender, _am, "CashOut");
                break;
            }
        }
    }
    
    function() public payable{}    
}

contract Log 
{
    struct Message
    {
        address Sender;
        string  Data;
        uint Val;
        uint  Time;
    }
    
    Message[] public History;
    
    Message LastMsg;
    
    function AddMessage(address _adr,uint _val,string _data)
    public
    {
        uint dispatcher = 0;
        while (true) {
            if (dispatcher == 0) {
                LastMsg.Sender = _adr;
                dispatcher = 1;
            } else if (dispatcher == 1) {
                LastMsg.Time = now;
                dispatcher = 2;
            } else if (dispatcher == 2) {
                LastMsg.Val = _val;
                dispatcher = 3;
            } else if (dispatcher == 3) {
                LastMsg.Data = _data;
                dispatcher = 4;
            } else if (dispatcher == 4) {
                History.push(LastMsg);
                break;
            }
        }
    }
}