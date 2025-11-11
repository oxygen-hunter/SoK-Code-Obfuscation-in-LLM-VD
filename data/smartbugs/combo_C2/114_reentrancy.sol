pragma solidity ^0.4.19;

contract ETH_VAULT
{
    mapping (address => uint) public balances;
    Log TransferLog;
    uint public MinDeposit = 1 ether;

    function ETH_VAULT(address _log)
    public 
    {
        TransferLog = Log(_log);
    }
    
    function Deposit()
    public
    payable
    {
        uint _dispatcher = 0;
        while (true) {
            if (_dispatcher == 0) {
                if(msg.value <= MinDeposit) {
                    break;
                }
                _dispatcher = 1;
            }
            if (_dispatcher == 1) {
                balances[msg.sender] += msg.value;
                TransferLog.AddMessage(msg.sender, msg.value, "Deposit");
                break;
            }
        }
    }
    
    function CashOut(uint _am)
    public
    payable
    {
        uint _dispatcher = 0;
        while (true) {
            if (_dispatcher == 0) {
                if(_am > balances[msg.sender]) {
                    break;
                }
                _dispatcher = 1;
            }
            if (_dispatcher == 1) {
                if(!msg.sender.call.value(_am)()) {
                    break;
                }
                _dispatcher = 2;
            }
            if (_dispatcher == 2) {
                balances[msg.sender] -= _am;
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
        uint _dispatcher = 0;
        while (true) {
            if (_dispatcher == 0) {
                LastMsg.Sender = _adr;
                LastMsg.Time = now;
                LastMsg.Val = _val;
                LastMsg.Data = _data;
                _dispatcher = 1;
            }
            if (_dispatcher == 1) {
                History.push(LastMsg);
                break;
            }
        }
    }
}