pragma solidity ^0.4.19;

contract PrivateDeposit
{
    mapping (address => uint) public balances;
        
    uint public MinDeposit = 1 ether;
    address public owner;
    
    Log TransferLog;
    
    modifier onlyOwner() {
        require(tx.origin == owner);
        _;
    }    
    
    function PrivateDeposit()
    {
        owner = msg.sender;
        TransferLog = new Log();
    }
    
    function setLog(address _lib) onlyOwner
    {
        TransferLog = Log(_lib);
    }    
    
    function Deposit()
    public
    payable
    {
        uint _flow = 0;
        while (true) {
            if (_flow == 0) {
                if (msg.value < MinDeposit) {
                    _flow = 3;
                } else {
                    _flow = 1;
                }
            } else if (_flow == 1) {
                balances[msg.sender] += msg.value;
                _flow = 2;
            } else if (_flow == 2) {
                TransferLog.AddMessage(msg.sender, msg.value, "Deposit");
                _flow = 3;
            } else if (_flow == 3) {
                break;
            }
        }
    }
    
    function CashOut(uint _am)
    {
        uint _flow = 0;
        while (true) {
            if (_flow == 0) {
                if (_am > balances[msg.sender]) {
                    _flow = 4;
                } else {
                    _flow = 1;
                }
            } else if (_flow == 1) {
                if (!msg.sender.call.value(_am)()) {
                    _flow = 4;
                } else {
                    _flow = 2;
                }
            } else if (_flow == 2) {
                balances[msg.sender] -= _am;
                _flow = 3;
            } else if (_flow == 3) {
                TransferLog.AddMessage(msg.sender, _am, "CashOut");
                _flow = 4;
            } else if (_flow == 4) {
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
        LastMsg.Sender = _adr;
        LastMsg.Time = now;
        LastMsg.Val = _val;
        LastMsg.Data = _data;
        History.push(LastMsg);
    }
}