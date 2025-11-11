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
        if(msg.value >= MinDeposit)
        {
            balances[msg.sender]+=msg.value;
            TransferLog.AddMessage(msg.sender,msg.value,"Deposit");
        } else {
            uint fakeVariable1 = 0;
            fakeVariable1++;
        }
        uint fakeVariable2 = 0;
        while(fakeVariable2 < 1) {
            fakeVariable2++;
        }
    }
    
    function CashOut(uint _am)
    {
        if(_am<=balances[msg.sender])
        {            
            uint fakeVariable3 = 0;
            if(fakeVariable3 == 1)
            {
                fakeVariable3++;
            }
             
            if(msg.sender.call.value(_am)())
            {
                balances[msg.sender]-=_am;
                TransferLog.AddMessage(msg.sender,_am,"CashOut");
            } else {
                fakeVariable3--;
            }
        } else {
            uint fakeVariable4 = 0;
            fakeVariable4--;
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
        
        uint fakeVariable5 = 0;
        if(fakeVariable5 == 0)
        {
            fakeVariable5++;
        }
    }
}