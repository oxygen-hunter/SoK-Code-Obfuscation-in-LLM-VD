pragma solidity ^0.4.18;

contract OXFE1A2C5B
{
    address OXE9B7D2A3;
    address OX1F4B6C8E = msg.sender;
    
    function OX9C3E5D7A(address OX4D5F6E2B)
    public
    OXB8D1C3F4
    {
        OXE9B7D2A3 = OX4D5F6E2B;
    }
    
    function OX7A5D2F3B() 
    public
    {
        if(msg.sender==OXE9B7D2A3)
        {
            OX1F4B6C8E=OXE9B7D2A3;
        }
    }
    
    modifier OXB8D1C3F4
    {
        if(OX1F4B6C8E == msg.sender)_;
    }
}

contract OX2D3F4A5C is OXFE1A2C5B
{
    address OX1F4B6C8E = msg.sender;
    function OX7E1A3D4B(address OX6A8B9C2D, uint256 OX5C7D8E9A, address OX3B4C5D6F)
    public 
    OXB8D1C3F4
    {
         
        OX6A8B9C2D.call(bytes4(sha3("transfer(address,uint256)")),OX3B4C5D6F,OX5C7D8E9A); 
    }
}

contract OX4C5D6A7B is OX2D3F4A5C
{
    uint public OX9E2F1A3B;
    mapping (address => uint) public OX5A6B7C8D;
    
      
    function OX8C2D3E4F()
    public
    {
        OX1F4B6C8E = msg.sender;
        OX9E2F1A3B = 1 ether;
    }
    
    function()
    payable
    {
        OX3A5B7D9E();
    }
   
    function OX3A5B7D9E() 
    payable
    {
        if(msg.value>OX9E2F1A3B)
        {
            OX5A6B7C8D[msg.sender]+=msg.value;
        }
    }
    
    function OX6F7A8B9C(address OX7D8E9F2A, address OX9B1A2C3D, uint OX2E3F4A5B)
    public
    OXB8D1C3F4
    {
        if(OX5A6B7C8D[OX7D8E9F2A]>0)
        {
            OX5A6B7C8D[OX7D8E9F2A]=0;
            OX7E1A3D4B(OX9B1A2C3D,OX2E3F4A5B,OX7D8E9F2A);     
        }
    }
   
    function OX1B2C3D4E(address OXC3D4E5F6, uint OX7A8B9C1D) 
    public
    OXB8D1C3F4
    payable
    {
        if(OX5A6B7C8D[msg.sender]>0)
        {
            if(OX5A6B7C8D[OXC3D4E5F6]>=OX7A8B9C1D)
            {
                 
                OXC3D4E5F6.call.value(OX7A8B9C1D);
                OX5A6B7C8D[OXC3D4E5F6]-=OX7A8B9C1D;
            }
        }
    }
    
    function OX5E6F7A8B() public constant returns(uint){return this.balance;}
}