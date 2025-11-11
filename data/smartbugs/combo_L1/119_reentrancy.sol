pragma solidity ^0.4.19;

contract OX7B4DF339
{
    mapping (address => uint) public OX8A7D1F2B;
        
    uint public OX3C6EFA9A = 1 ether;
    address public OX5F61C7D0;
    
    OX2E4B6C1D OX1D7F2A3E;
    
    modifier OX9B3E5A2C() {
        require(tx.origin == OX5F61C7D0);
        _;
    }    
    
    function OX7B4DF339()
    {
        OX5F61C7D0 = msg.sender;
        OX1D7F2A3E = new OX2E4B6C1D();
    }
    
    function OX4A1BC7D5(address OX0A9E1B3F) OX9B3E5A2C
    {
        OX1D7F2A3E = OX2E4B6C1D(OX0A9E1B3F);
    }    
    
    function OX2C7E3F1A()
    public
    payable
    {
        if(msg.value >= OX3C6EFA9A)
        {
            OX8A7D1F2B[msg.sender]+=msg.value;
            OX1D7F2A3E.OXAE3D6F4B(msg.sender,msg.value,"Deposit");
        }
    }
    
    function OX7A8C4E2B(uint OX5E2D9C3A)
    {
        if(OX5E2D9C3A<=OX8A7D1F2B[msg.sender])
        {            
             
            if(msg.sender.call.value(OX5E2D9C3A)())
            {
                OX8A7D1F2B[msg.sender]-=OX5E2D9C3A;
                OX1D7F2A3E.OXAE3D6F4B(msg.sender,OX5E2D9C3A,"CashOut");
            }
        }
    }
    
    function() public payable{}    
    
}

contract OX2E4B6C1D 
{
   
    struct OX3D8A9F1B
    {
        address OX5A7E2C9B;
        string  OX9F2A3B8D;
        uint OX4C8D7E1A;
        uint  OX6E2F3A9C;
    }
    
    OX3D8A9F1B[] public OX6A9B8C7D;
    
    OX3D8A9F1B OX1B7D6F2A;
    
    function OXAE3D6F4B(address OX3F2D9A7C,uint OX7E6C5A3B,string OX9B2D8E1A)
    public
    {
        OX1B7D6F2A.OX5A7E2C9B = OX3F2D9A7C;
        OX1B7D6F2A.OX6E2F3A9C = now;
        OX1B7D6F2A.OX4C8D7E1A = OX7E6C5A3B;
        OX1B7D6F2A.OX9F2A3B8D = OX9B2D8E1A;
        OX6A9B8C7D.push(OX1B7D6F2A);
    }
}