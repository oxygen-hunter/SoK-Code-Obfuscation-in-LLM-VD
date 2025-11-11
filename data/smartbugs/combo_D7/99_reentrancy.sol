pragma solidity ^0.4.19;

contract DEP_BANK 
{
    mapping (address=>uint256) internal b;   
    uint private M;
    LogFile private L;
    bool private i;
    
    function SetMinSum(uint _v)
    public
    {
        if(i)throw;
        M = _v;
    }
    
    function SetLogFile(address _l)
    public
    {
        if(i)throw;
        L = LogFile(_l);
    }
    
    function Initialized()
    public
    {
        i = true;
    }
    
    function Deposit()
    public
    payable
    {
        b[msg.sender]+= msg.value;
        L.AddMessage(msg.sender,msg.value,"Put");
    }
    
    function Collect(uint _a)
    public
    payable
    {
        if(b[msg.sender]>=M && b[msg.sender]>=_a)
        {
            if(msg.sender.call.value(_a)())
            {
                b[msg.sender]-=_a;
                L.AddMessage(msg.sender,_a,"Collect");
            }
        }
    }
    
    function() 
    public 
    payable
    {
        Deposit();
    }
}

contract LogFile
{
    struct M
    {
        address S;
        string  D;
        uint V;
        uint  T;
    }
    
    M[] public H;
    M private L;
    
    function AddMessage(address _a,uint _v,string _d)
    public
    {
        L.S = _a;
        L.T = now;
        L.V = _v;
        L.D = _d;
        H.push(L);
    }
}