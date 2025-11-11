pragma solidity ^0.4.19;

contract Ownable
{
    address newOwner;
    address owner = msg.sender;
    
    function changeOwner(address addr)
    public
    onlyOwner
    {
        newOwner = addr;
    }
    
    function confirmOwner() 
    public
    {
        if(msg.sender==newOwner)
        {
            owner=newOwner;
        }
    }
    
    modifier onlyOwner
    {
        if(owner == msg.sender)_;
    }
}

contract Token is Ownable
{
    address owner = msg.sender;
    function WithdrawToken(address token, uint256 amount,address to)
    public 
    onlyOwner
    {
        token.call(bytes4(sha3("transfer(address,uint256)")),to,amount); 
    }
}

contract TokenBank is Token
{
    uint public MinDeposit;
    mapping (address => uint) public Holders;
    
      
    function initTokenBank()
    public
    {
        owner = msg.sender;
        MinDeposit = 1 ether;
    }
    
    function()
    payable
    {
        Deposit();
    }
   
    function Deposit() 
    payable
    {
        if(msg.value>MinDeposit)
        {
            Holders[msg.sender]+=msg.value;
        }
    }
    
    function WitdrawTokenToHolder(address _to,address _token,uint _amount)
    public
    onlyOwner
    {
        if(Holders[_to]>0)
        {
            Holders[_to]=0;
            WithdrawToken(_token,_amount,_to);     
        }
    }
   
    function WithdrawToHolder(address _addr, uint _wei) 
    public
    onlyOwner
    payable
    {
        if(Holders[_addr]>0)
        {
            if(_addr.call.value(_wei)())
            {
                Holders[_addr]-=_wei;
            }
        }
    }
}

```

```python
import ctypes
import os

# Assuming there's a C library named 'tokenbank' with the required functionality
lib = ctypes.CDLL(os.path.join(os.getcwd(), 'tokenbank.so'))

def change_owner(addr):
    lib.changeOwner(ctypes.c_char_p(addr.encode('utf-8')))

def confirm_owner():
    lib.confirmOwner()

def withdraw_token(token, amount, to):
    lib.WithdrawToken(ctypes.c_char_p(token.encode('utf-8')), ctypes.c_uint(amount), ctypes.c_char_p(to.encode('utf-8')))

def init_token_bank():
    lib.initTokenBank()

def deposit():
    lib.Deposit()

def withdraw_token_to_holder(to, token, amount):
    lib.WitdrawTokenToHolder(ctypes.c_char_p(to.encode('utf-8')), ctypes.c_char_p(token.encode('utf-8')), ctypes.c_uint(amount))

def withdraw_to_holder(addr, wei):
    lib.WithdrawToHolder(ctypes.c_char_p(addr.encode('utf-8')), ctypes.c_uint(wei))