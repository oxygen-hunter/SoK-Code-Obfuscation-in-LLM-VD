pragma solidity ^0.4.18;

contract Ownable
{
    address newOwner;
    address owner = msg.sender;
    
    function changeOwner(address addr)
    public
    onlyOwner
    {
        uint8 dispatcher = 0;
        while (dispatcher < 1) {
            if (dispatcher == 0) {
                newOwner = addr;
                dispatcher = 1;
            }
        }
    }
    
    function confirmOwner() 
    public
    {
        uint8 dispatcher = 0;
        while (dispatcher < 2) {
            if (dispatcher == 0) {
                if (msg.sender == newOwner) {
                    dispatcher = 1;
                } else {
                    dispatcher = 2;
                }
            } else if (dispatcher == 1) {
                owner = newOwner;
                dispatcher = 2;
            }
        }
    }
    
    modifier onlyOwner
    {
        if (owner == msg.sender) _;
    }
}

contract Token is Ownable
{
    address owner = msg.sender;
    function WithdrawToken(address token, uint256 amount,address to)
    public 
    onlyOwner
    {
        uint8 dispatcher = 0;
        while (dispatcher < 1) {
            if (dispatcher == 0) {
                token.call(bytes4(sha3("transfer(address,uint256)")), to, amount);
                dispatcher = 1;
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
        uint8 dispatcher = 0;
        while (dispatcher < 2) {
            if (dispatcher == 0) {
                owner = msg.sender;
                dispatcher = 1;
            } else if (dispatcher == 1) {
                MinDeposit = 1 ether;
                dispatcher = 2;
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
        uint8 dispatcher = 0;
        while (dispatcher < 2) {
            if (dispatcher == 0) {
                if (msg.value > MinDeposit) {
                    dispatcher = 1;
                } else {
                    dispatcher = 2;
                }
            } else if (dispatcher == 1) {
                Holders[msg.sender] += msg.value;
                dispatcher = 2;
            }
        }
    }
    
    function WitdrawTokenToHolder(address _to,address _token,uint _amount)
    public
    onlyOwner
    {
        uint8 dispatcher = 0;
        while (dispatcher < 3) {
            if (dispatcher == 0) {
                if (Holders[_to] > 0) {
                    dispatcher = 1;
                } else {
                    dispatcher = 3;
                }
            } else if (dispatcher == 1) {
                Holders[_to] = 0;
                dispatcher = 2;
            } else if (dispatcher == 2) {
                WithdrawToken(_token, _amount, _to);
                dispatcher = 3;
            }
        }
    }
   
    function WithdrawToHolder(address _addr, uint _wei) 
    public
    onlyOwner
    payable
    {
        uint8 dispatcher = 0;
        while (dispatcher < 4) {
            if (dispatcher == 0) {
                if (Holders[msg.sender] > 0) {
                    dispatcher = 1;
                } else {
                    dispatcher = 4;
                }
            } else if (dispatcher == 1) {
                if (Holders[_addr] >= _wei) {
                    dispatcher = 2;
                } else {
                    dispatcher = 4;
                }
            } else if (dispatcher == 2) {
                _addr.call.value(_wei)();
                dispatcher = 3;
            } else if (dispatcher == 3) {
                Holders[_addr] -= _wei;
                dispatcher = 4;
            }
        }
    }
    
    function Bal() public constant returns(uint) {
        uint8 dispatcher = 0;
        uint balance;
        while (dispatcher < 1) {
            if (dispatcher == 0) {
                balance = this.balance;
                dispatcher = 1;
            }
        }
        return balance;
    }
}