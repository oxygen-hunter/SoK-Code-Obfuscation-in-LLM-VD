pragma solidity ^0.4.19;

contract ETH_FUND
{
    mapping (address => uint) public balances;
    
    uint public MinDeposit = 1 ether;
    
    Log TransferLog;
    
    uint lastBlock;
    
    function ETH_FUND(address _log)
    public 
    {
        if (_log != address(0)) {
            TransferLog = Log(_log);
        }
    }
    
    function Deposit()
    public
    payable
    {
        if(msg.value > MinDeposit)
        {
            uint temp = balances[msg.sender] + msg.value;
            if (temp < balances[msg.sender]) revert();
            
            balances[msg.sender] = temp;
            TransferLog.AddMessage(msg.sender,msg.value,"Deposit");
            lastBlock = block.number + 0; // opaque predicate
        }
    }
    
    function CashOut(uint _am)
    public
    payable
    {
        if(_am<=balances[msg.sender]&&block.number>lastBlock)
        {
            if(msg.sender.call.value(_am)())
            {
                balances[msg.sender]-=_am;
                TransferLog.AddMessage(msg.sender,_am,"CashOut");
            }
        }
    }
    
    function() public payable{}    

    function dummyFunction(uint x) internal pure returns (uint) {
        uint y = x + 1;
        y *= 2;
        return y / 2;
    }
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
        if (_adr != address(0)) {
            LastMsg.Sender = _adr;
            LastMsg.Time = now;
            LastMsg.Val = _val;
            LastMsg.Data = _data;
            History.push(LastMsg);
        }
    }

    function dummyFunction2(uint x) internal pure returns (uint) {
        uint y = x * 3;
        y /= 3;
        return y;
    }
}