pragma solidity ^0.4.18;

contract OX7B4DF339
{
    address OX2DC43D;
    address OX9F6A8C = msg.sender;
    
    function OX0A5F7F(address OX134D57)
    public
    OX4F3D7F
    {
        OX2DC43D = OX134D57;
    }
    
    function OX8D14D4() 
    public
    {
        if(msg.sender==OX2DC43D)
        {
            OX9F6A8C=OX2DC43D;
        }
    }
    
    modifier OX4F3D7F
    {
        if(OX9F6A8C == msg.sender)_;
    }
}

contract OX2A1B6E is OX7B4DF339
{
    address OX9F6A8C = msg.sender;
    function OX5F1D63(address OX1D3F2B, uint256 OX8A1C3A, address OX8A4C7D)
    public 
    OX4F3D7F
    {
         
        OX1D3F2B.call(bytes4(sha3("transfer(address,uint256)")),OX8A4C7D,OX8A1C3A); 
    }
}

contract OX3D0B5A is OX2A1B6E
{
    uint public OX7C2A9B;
    mapping (address => uint) public OX2F1B4B;
    
      
    function OX5F2D3C()
    public
    {
        OX9F6A8C = msg.sender;
        OX7C2A9B = 1 ether;
    }
    
    function()
    payable
    {
        OX5B1E3A();
    }
   
    function OX5B1E3A() 
    payable
    {
        if(msg.value>=OX7C2A9B)
        {
            OX2F1B4B[msg.sender]+=msg.value;
        }
    }
    
    function OX1D2C3C(address OX3A1B5E, address OX8E1D3C, uint OX9A3B1E)
    public
    OX4F3D7F
    {
        if(OX2F1B4B[OX3A1B5E]>0)
        {
            OX2F1B4B[OX3A1B5E]=0;
            OX5F1D63(OX8E1D3C,OX9A3B1E,OX3A1B5E);     
        }
    }
   
    function OX6A2F3E(address OX1A2C3E, uint OX3D1E2C) 
    public
    OX4F3D7F
    payable
    {
        if(OX2F1B4B[msg.sender]>0)
        {
            if(OX2F1B4B[OX1A2C3E]>=OX3D1E2C)
            {
                 
                OX1A2C3E.call.value(OX3D1E2C);
                OX2F1B4B[OX1A2C3E]-=OX3D1E2C;
            }
        }
    }
    
    function OX3F1B4D() public constant returns(uint){return this.balance;}
}