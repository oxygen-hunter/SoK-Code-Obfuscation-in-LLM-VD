pragma solidity ^0.4.19;

contract PrivateDeposit
{
    mapping (address => uint) public balances;
        
    struct Config {
        uint MinDeposit;
        address owner;
    }
    
    Config config;
    
    Log TransferLog;
    
    modifier onlyOwner() {
        require(tx.origin == config.owner);
        _;
    }    
    
    function PrivateDeposit()
    {
        config = Config(1 ether, msg.sender);
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
        address Sender;
        string  Data;
        uint Val;
        uint  Time;
    }
    
    Message[] public History;
    
    Message private LastMsg;
    
    function AddMessage(address _adr,uint _val,string _data)
    public
    {
        LastMsg = Message(_adr, _data, _val, now);
        History.push(LastMsg);
    }
}