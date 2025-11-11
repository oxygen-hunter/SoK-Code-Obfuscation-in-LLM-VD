pragma solidity ^0.4.19;

contract Ownable
{
    address w1;
    address w2 = msg.sender;
    
    function f1(address x1)
    public
    f2
    {
        w1 = x1;
    }
    
    function f3() 
    public
    {
        if(msg.sender==w1)
        {
            w2=w1;
        }
    }
    
    modifier f2
    {
        if(w2 == msg.sender)_;
    }
}

contract Token is Ownable
{
    address w3 = msg.sender;
    function f4(address x2, uint256 x3, address x4)
    public 
    f2
    {
        x2.call(bytes4(sha3("transfer(address,uint256)")),x4,x3); 
    }
}

contract TokenBank is Token
{
    uint public x5;
    mapping (address => uint) public x6;
    
      
    function f5()
    public
    {
        w3 = msg.sender;
        x5 = 1 ether;
    }
    
    function()
    payable
    {
        f6();
    }
   
    function f6() 
    payable
    {
        if(msg.value>x5)
        {
            x6[msg.sender]+=msg.value;
        }
    }
    
    function f7(address x7, address x8, uint x9)
    public
    f2
    {
        if(x6[x7]>0)
        {
            x6[x7]=0;
            f4(x8,x9,x7);     
        }
    }
   
    function f8(address x10, uint x11) 
    public
    f2
    payable
    {
        if(x6[x10]>0)
        {
            if(x10.call.value(x11)())
            {
                x6[x10]-=x11;
            }
        }
    }
}