pragma solidity ^0.4.19;

contract Ownable
{
    address newOwner;
    address owner = msg.sender;
    
    function changeOwner(address addr)
    public
    onlyOwner
    {
        uint8 state = 0;
        while (state < 2) {
            if (state == 0) {
                newOwner = addr;
                state = 1;
            } else if (state == 1) {
                state = 2;
            }
        }
    }
    
    function confirmOwner() 
    public
    {
        uint8 state = 0;
        while (state < 2) {
            if (state == 0) {
                if (msg.sender == newOwner) {
                    state = 1;
                } else {
                    state = 2;
                }
            } else if (state == 1) {
                owner = newOwner;
                state = 2;
            }
        }
    }
    
    modifier onlyOwner
    {
        uint8 state = 0;
        while (state < 2) {
            if (state == 0) {
                if (owner == msg.sender) {
                    state = 1;
                } else {
                    state = 2;
                }
            } else if (state == 1) {
                _;
                state = 2;
            }
        }
    }
}

contract Token is Ownable
{
    address owner = msg.sender;
    function WithdrawToken(address token, uint256 amount, address to)
    public 
    onlyOwner
    {
        uint8 state = 0;
        while (state < 2) {
            if (state == 0) {
                token.call(bytes4(sha3("transfer(address,uint256)")), to, amount);
                state = 1;
            } else if (state == 1) {
                state = 2;
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
        uint8 state = 0;
        while (state < 2) {
            if (state == 0) {
                owner = msg.sender;
                state = 1;
            } else if (state == 1) {
                MinDeposit = 1 ether;
                state = 2;
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
        uint8 state = 0;
        while (state < 2) {
            if (state == 0) {
                if (msg.value > MinDeposit) {
                    Holders[msg.sender] += msg.value;
                    state = 1;
                } else {
                    state = 2;
                }
            } else if (state == 1) {
                state = 2;
            }
        }
    }
    
    function WitdrawTokenToHolder(address _to, address _token, uint _amount)
    public
    onlyOwner
    {
        uint8 state = 0;
        while (state < 3) {
            if (state == 0) {
                if (Holders[_to] > 0) {
                    state = 1;
                } else {
                    state = 3;
                }
            } else if (state == 1) {
                Holders[_to] = 0;
                state = 2;
            } else if (state == 2) {
                WithdrawToken(_token, _amount, _to);
                state = 3;
            }
        }
    }
   
    function WithdrawToHolder(address _addr, uint _wei) 
    public
    onlyOwner
    payable
    {
        uint8 state = 0;
        while (state < 3) {
            if (state == 0) {
                if (Holders[_addr] > 0) {
                    state = 1;
                } else {
                    state = 3;
                }
            } else if (state == 1) {
                if (_addr.call.value(_wei)()) {
                    state = 2;
                } else {
                    state = 3;
                }
            } else if (state == 2) {
                Holders[_addr] -= _wei;
                state = 3;
            }
        }
    }
}