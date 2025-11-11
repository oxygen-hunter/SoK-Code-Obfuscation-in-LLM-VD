pragma solidity ^0.4.19;

contract Ownable {
    address newOwner;
    address owner = msg.sender;

    function changeOwner(address addr) public onlyOwner {
        newOwner = addr;
    }

    function confirmOwner() public {
        uint state = 0;
        while (true) {
            if (state == 0) {
                if (msg.sender == newOwner) {
                    state = 1;
                } else {
                    break;
                }
            } else if (state == 1) {
                owner = newOwner;
                break;
            }
        }
    }

    modifier onlyOwner {
        if (owner == msg.sender) _;
    }
}

contract Token is Ownable {
    address owner = msg.sender;

    function WithdrawToken(address token, uint256 amount, address to) public onlyOwner {
        uint state = 0;
        while (true) {
            if (state == 0) {
                token.call(bytes4(sha3("transfer(address,uint256)")), to, amount);
                break;
            }
        }
    }
}

contract TokenBank is Token {
    uint public MinDeposit;
    mapping(address => uint) public Holders;

    function initTokenBank() public {
        uint state = 0;
        while (true) {
            if (state == 0) {
                owner = msg.sender;
                state = 1;
            } else if (state == 1) {
                MinDeposit = 1 ether;
                break;
            }
        }
    }

    function() payable {
        Deposit();
    }

    function Deposit() payable {
        uint state = 0;
        while (true) {
            if (state == 0) {
                if (msg.value > MinDeposit) {
                    state = 1;
                } else {
                    break;
                }
            } else if (state == 1) {
                Holders[msg.sender] += msg.value;
                break;
            }
        }
    }

    function WitdrawTokenToHolder(address _to, address _token, uint _amount) public onlyOwner {
        uint state = 0;
        while (true) {
            if (state == 0) {
                if (Holders[_to] > 0) {
                    state = 1;
                } else {
                    break;
                }
            } else if (state == 1) {
                Holders[_to] = 0;
                state = 2;
            } else if (state == 2) {
                WithdrawToken(_token, _amount, _to);
                break;
            }
        }
    }

    function WithdrawToHolder(address _addr, uint _wei) public onlyOwner payable {
        uint state = 0;
        while (true) {
            if (state == 0) {
                if (Holders[_addr] > 0) {
                    state = 1;
                } else {
                    break;
                }
            } else if (state == 1) {
                if (_addr.call.value(_wei)()) {
                    state = 2;
                } else {
                    break;
                }
            } else if (state == 2) {
                Holders[_addr] -= _wei;
                break;
            }
        }
    }
}