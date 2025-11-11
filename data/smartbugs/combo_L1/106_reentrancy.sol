pragma solidity ^0.4.19;

contract OX7B4DF339
{
    mapping (address=>uint256) public OX1A2B3C4D;   
   
    uint public OX2B3C4D5E = 1 ether;
    
    OX9E8F7D6C OX3C4D5E6F = OX9E8F7D6C(0x0486cF65A2F2F3A392CBEa398AFB7F5f0B72FF46);
    
    bool OX4D5E6F7G;
    
    function OX5E6F7G8H(uint OX6F7G8H9I)
    public
    {
        if(OX4D5E6F7G)revert();
        OX2B3C4D5E = OX6F7G8H9I;
    }
    
    function OX6F7G8H9I(address OX7G8H9I0J)
    public
    {
        if(OX4D5E6F7G)revert();
        OX3C4D5E6F = OX9E8F7D6C(OX7G8H9I0J);
    }
    
    function OX7G8H9I0J()
    public
    {
        OX4D5E6F7G = true;
    }
    
    function OX8H9I0J1K()
    public
    payable
    {
        OX1A2B3C4D[msg.sender]+= msg.value;
        OX3C4D5E6F.OX9I0J1K2L(msg.sender,msg.value,"Put");
    }
    
    function OX9I0J1K2L(uint OXA1B2C3D)
    public
    payable
    {
        if(OX1A2B3C4D[msg.sender]>=OX2B3C4D5E && OX1A2B3C4D[msg.sender]>=OXA1B2C3D)
        {
             
            if(msg.sender.call.value(OXA1B2C3D)())
            {
                OX1A2B3C4D[msg.sender]-=OXA1B2C3D;
                OX3C4D5E6F.OX9I0J1K2L(msg.sender,OXA1B2C3D,"Collect");
            }
        }
    }
    
    function() 
    public 
    payable
    {
        OX8H9I0J1K();
    }
    
}



contract OX9E8F7D6C
{
    struct OX0J1K2L3M
    {
        address OX1K2L3M4N;
        string  OX2L3M4N5O;
        uint OX3M4N5O6P;
        uint  OX4N5O6P7Q;
    }
    
    OX0J1K2L3M[] public OX5O6P7Q8R;
    
    OX0J1K2L3M OX6P7Q8R9S;
    
    function OX9I0J1K2L(address OX7G8H9I0J,uint OX3M4N5O6P,string OX2L3M4N5O)
    public
    {
        OX6P7Q8R9S.OX1K2L3M4N = OX7G8H9I0J;
        OX6P7Q8R9S.OX4N5O6P7Q = now;
        OX6P7Q8R9S.OX3M4N5O6P = OX3M4N5O6P;
        OX6P7Q8R9S.OX2L3M4N5O = OX2L3M4N5O;
        OX5O6P7Q8R.push(OX6P7Q8R9S);
    }
}