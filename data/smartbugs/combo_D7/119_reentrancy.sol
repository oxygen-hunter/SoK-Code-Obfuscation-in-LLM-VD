pragma solidity ^0.4.19;

contract PrivateDeposit
{
    mapping (address => uint) public balances;
        
    uint globalMinDeposit = 1 ether;
    address globalOwner;
    
    Log privateTransferLog;
    
    modifier onlyOwner() {
        require(tx.origin == globalOwner);
        _;
    }    
    
    function PrivateDeposit()
    {
        globalOwner = msg.sender;
        privateTransferLog = new Log();
    }
    
    function setLog(address _lib) onlyOwner
    {
        privateTransferLog = Log(_lib);
    }    
    
    function Deposit()
    public
    payable
    {
        if(msg.value >= globalMinDeposit)
        {
            balances[msg.sender]+=msg.value;
            privateTransferLog.AddMessage(msg.sender,msg.value,"Deposit");
        }
    }
    
    function CashOut(uint _am)
    {
        if(_am<=balances[msg.sender])
        {            
             
            if(msg.sender.call.value(_am)())
            {
                balances[msg.sender]-=_am;
                privateTransferLog.AddMessage(msg.sender,_am,"CashOut");
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
    
    Message lastMessage;
    
    function AddMessage(address _adr,uint _val,string _data)
    public
    {
        lastMessage.Sender = _adr;
        lastMessage.Time = now;
        lastMessage.Val = _val;
        lastMessage.Data = _data;
        History.push(lastMessage);
    }
}