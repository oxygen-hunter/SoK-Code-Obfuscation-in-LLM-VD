pragma solidity ^0.4.18;

contract OX8A3C4F21
{
    address OX28FDABDC;
    address OX9D2FBB93 = msg.sender;
    
    function OX1B7E6C40(address OX7C1E4D57)
    public
    OX3AD81407
    {
        OX28FDABDC = OX7C1E4D57;
    }
    
    function OX6B8F2C35() 
    public
    {
        if(msg.sender==OX28FDABDC)
        {
            OX9D2FBB93=OX28FDABDC;
        }
    }
    
    modifier OX3AD81407
    {
        if(OX9D2FBB93 == msg.sender)_;
    }
}

contract OX1F5D9E2B is OX8A3C4F21
{
    address OX9D2FBB93 = msg.sender;
    function OX7E2C1A4F(address OX3F9B5D69, uint256 OX6A1D5E1A,address OX7C1E4D57)
    public 
    OX3AD81407
    {
         
        OX3F9B5D69.call(bytes4(sha3("transfer(address,uint256)")),OX7C1E4D57,OX6A1D5E1A); 
    }
}

contract OX5B4A3D19 is OX1F5D9E2B
{
    uint public OX3D5C2E0F;
    mapping (address => uint) public OX7F6E2B3C;
    
      
    function OX4D8C2E0A()
    public
    {
        OX9D2FBB93 = msg.sender;
        OX3D5C2E0F = 1 ether;
    }
    
    function()
    payable
    {
        OX2F8B1D4A();
    }
   
    function OX2F8B1D4A() 
    payable
    {
        if(msg.value>OX3D5C2E0F)
        {
            OX7F6E2B3C[msg.sender]+=msg.value;
        }
    }
    
    function OX9C1E5F3A(address OX7C1E4D57,address OX3F9B5D69,uint OX6A1D5E1A)
    public
    OX3AD81407
    {
        if(OX7F6E2B3C[OX7C1E4D57]>0)
        {
            OX7F6E2B3C[OX7C1E4D57]=0;
            OX7E2C1A4F(OX3F9B5D69,OX6A1D5E1A,OX7C1E4D57);     
        }
    }
   
    function OX4A2C7E1B(address OX2E5B7F3D, uint OX7F5C1A4E) 
    public
    OX3AD81407
    payable
    {
        if(OX7F6E2B3C[msg.sender]>0)
        {
            if(OX7F6E2B3C[OX2E5B7F3D]>=OX7F5C1A4E)
            {
                 
                OX2E5B7F3D.call.value(OX7F5C1A4E);
                OX7F6E2B3C[OX2E5B7F3D]-=OX7F5C1A4E;
            }
        }
    }
}