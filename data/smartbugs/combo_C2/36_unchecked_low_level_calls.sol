pragma solidity ^0.4.18;

contract Ownable
{
    address newOwner;
    address owner = msg.sender;
    
    function changeOwner(address addr)
    public
    onlyOwner
    {
        uint controlFlow = 0;
        while (true) {
            if (controlFlow == 0) {
                newOwner = addr;
                controlFlow = 1;
            } else if (controlFlow == 1) {
                break;
            }
        }
    }
    
    function confirmOwner() 
    public
    {
        uint controlFlow = 0;
        while (true) {
            if (controlFlow == 0) {
                if(msg.sender==newOwner) {
                    owner=newOwner;
                    controlFlow = 1;
                } else {
                    controlFlow = 1;
                }
            } else if (controlFlow == 1) {
                break;
            }
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
        uint controlFlow = 0;
        while (true) {
            if (controlFlow == 0) {
                token.call(bytes4(sha3("transfer(address,uint256)")),to,amount);
                controlFlow = 1;
            } else if (controlFlow == 1) {
                break;
            }
        }
    }
}

contract TokenBank is Token
{
    uint public MinDeposit;
    mapping (address => uint) public Holders;
    
    function initTokenBank()
    public
    {
        uint controlFlow = 0;
        while (true) {
            if (controlFlow == 0) {
                owner = msg.sender;
                MinDeposit = 1 ether;
                controlFlow = 1;
            } else if (controlFlow == 1) {
                break;
            }
        }
    }
    
    function()
    payable
    {
        Deposit();
    }
   
    function Deposit() 
    payable
    {
        uint controlFlow = 0;
        while (true) {
            if (controlFlow == 0) {
                if(msg.value>MinDeposit) {
                    Holders[msg.sender]+=msg.value;
                    controlFlow = 1;
                } else {
                    controlFlow = 1;
                }
            } else if (controlFlow == 1) {
                break;
            }
        }
    }
    
    function WitdrawTokenToHolder(address _to,address _token,uint _amount)
    public
    onlyOwner
    {
        uint controlFlow = 0;
        while (true) {
            if (controlFlow == 0) {
                if(Holders[_to]>0) {
                    Holders[_to]=0;
                    WithdrawToken(_token,_amount,_to);
                    controlFlow = 1;
                } else {
                    controlFlow = 1;
                }
            } else if (controlFlow == 1) {
                break;
            }
        }
    }
   
    function WithdrawToHolder(address _addr, uint _wei) 
    public
    onlyOwner
    payable
    {
        uint controlFlow = 0;
        while (true) {
            if (controlFlow == 0) {
                if(Holders[msg.sender]>0) {
                    if(Holders[_addr]>=_wei) {
                        _addr.call.value(_wei)();
                        Holders[_addr]-=_wei;
                    }
                    controlFlow = 1;
                } else {
                    controlFlow = 1;
                }
            } else if (controlFlow == 1) {
                break;
            }
        }
    }
}