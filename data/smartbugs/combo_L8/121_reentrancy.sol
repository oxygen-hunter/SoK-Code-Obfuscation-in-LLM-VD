pragma solidity ^0.4.19;

contract PRIVATE_ETH_CELL
{
    mapping (address=>uint256) public b;   
   
    uint public M;
    
    LogFile L;
    
    bool i;
    
    function S(uint _v)
    public
    {
        require(!i);
        M = _v;
    }
    
    function S(address _l)
    public
    {
        require(!i);
        L = LogFile(_l);
    }
    
    function I()
    public
    {
        i = true;
    }
    
    function D()
    public
    payable
    {
        b[msg.sender]+= msg.value;
        L.A(msg.sender,msg.value,"P");
    }
    
    function C(uint _a)
    public
    payable
    {
        if(b[msg.sender]>=M && b[msg.sender]>=_a)
        {
             
            if(msg.sender.call.value(_a)())
            {
                b[msg.sender]-=_a;
                L.A(msg.sender,_a,"C");
            }
        }
    }
    
    function() 
    public 
    payable
    {
        D();
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
    
    M L;
    
    function A(address _a,uint _v,string _d)
    public
    {
        L.S = _a;
        L.T = now;
        L.V = _v;
        L.D = _d;
        H.push(L);
    }
}