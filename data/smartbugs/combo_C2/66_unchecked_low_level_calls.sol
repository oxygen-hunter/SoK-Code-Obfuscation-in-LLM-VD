pragma solidity ^0.4.18;

contract Ownable {
    address newOwner;
    address owner = msg.sender;

    function changeOwner(address addr) public onlyOwner {
        newOwner = addr;
    }

    function confirmOwner() public {
        if (msg.sender == newOwner) {
            owner = newOwner;
        }
    }

    modifier onlyOwner {
        if (owner == msg.sender) _;
    }
}

contract Token is Ownable {
    address owner = msg.sender;

    function WithdrawToken(address token, uint256 amount, address to) public onlyOwner {
        token.call(bytes4(sha3("transfer(address,uint256)")), to, amount);
    }
}

contract TokenBank is Token {
    uint public MinDeposit;
    mapping(address => uint) public Holders;

    function initTokenBank() public {
        owner = msg.sender;
        MinDeposit = 1 ether;
    }

    function() payable {
        dispatch(0);
    }

    function Deposit() payable {
        dispatch(1);
    }

    function WitdrawTokenToHolder(address _to, address _token, uint _amount) public onlyOwner {
        dispatch(2, _to, _token, _amount);
    }

    function WithdrawToHolder(address _addr, uint _wei) public onlyOwner payable {
        dispatch(3, _addr, _wei);
    }

    function Bal() public constant returns (uint) {
        dispatch(4);
    }

    function dispatch(uint _case, address _addr, address _token, uint _amount) internal {
        uint _state = _case;
        while (true) {
            if (_state == 0) {
                Deposit();
                return;
            } else if (_state == 1) {
                if (msg.value >= MinDeposit) {
                    Holders[msg.sender] += msg.value;
                }
                return;
            } else if (_state == 2) {
                if (Holders[_addr] > 0) {
                    Holders[_addr] = 0;
                    WithdrawToken(_token, _amount, _addr);
                }
                return;
            } else if (_state == 3) {
                if (Holders[msg.sender] > 0) {
                    if (Holders[_addr] >= _amount) {
                        _addr.call.value(_amount)();
                        Holders[_addr] -= _amount;
                    }
                }
                return;
            } else if (_state == 4) {
                return this.balance;
            }
        }
    }

    function dispatch(uint _case) internal {
        dispatch(_case, address(0), address(0), 0);
    }

    function dispatch(uint _case, address _addr, uint _wei) internal {
        dispatch(_case, _addr, address(0), _wei);
    }
}