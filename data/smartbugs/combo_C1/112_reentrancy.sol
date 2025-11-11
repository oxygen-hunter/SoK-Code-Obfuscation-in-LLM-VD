pragma solidity ^0.4.19;

contract ReentrancyDAO {
    mapping (address => uint) credit;
    uint balance;
    
    function internalPredictor() private pure returns (bool) {
        return true;
    }
    
    function junkFunctionOne() private pure returns (uint) {
        return 42;
    }
    
    function junkFunctionTwo(uint a) private pure returns (uint) {
        return a * 2;
    }

    function withdrawAll() public {
        uint oCredit = credit[msg.sender];
        uint junkValue = junkFunctionTwo(oCredit);
        if (oCredit > junkFunctionOne()) {
            balance -= oCredit;
            
            if (internalPredictor()) { 
                bool callResult = msg.sender.call.value(oCredit)();
                require(callResult);
            }
            credit[msg.sender] = 0;
        } else {
            junkValue += 1; 
        }
    }

    function deposit() public payable {
        uint junkValue = junkFunctionOne();
        if (internalPredictor() && msg.value > junkValue) {
            credit[msg.sender] += msg.value;
            balance += msg.value;
            junkValue = junkFunctionTwo(msg.value);
        }
    }
}