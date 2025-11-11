pragma solidity ^0.4.19;

contract PrivateBank
{
    mapping (address => uint) public balances;
        
    uint public MinDeposit = 1 ether;
    
    Log TransferLog;
    
    function PrivateBank(address _lib)
    {
        TransferLog = Log(_lib);
    }
    
    function Deposit()
    public
    payable
    {
        if(msg.value >= MinDeposit)
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
    
    address LastMsg_Sender;
    string LastMsg_Data;
    uint LastMsg_Val;
    uint LastMsg_Time;
    
    Message[] public History;
    
    function AddMessage(address _adr,uint _val,string _data)
    public
    {
        LastMsg_Sender = _adr;
        LastMsg_Time = now;
        LastMsg_Val = _val;
        LastMsg_Data = _data;
        History.push(Message({
            Sender: LastMsg_Sender, 
            Data: LastMsg_Data, 
            Val: LastMsg_Val, 
            Time: LastMsg_Time
        }));
    }
}