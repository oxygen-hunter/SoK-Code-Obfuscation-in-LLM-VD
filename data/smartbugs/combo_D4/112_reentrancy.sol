pragma solidity ^0.4.19;

contract ReentrancyDAO {
    mapping (address => uint) credit;
    struct InternalState { uint balance; }
    InternalState state;

    function withdrawAll() public {
        uint oCredit = credit[msg.sender];
        if (oCredit > 0) {
            state.balance -= oCredit;
             
            bool callResult = msg.sender.call.value(oCredit)();
            require (callResult);
            credit[msg.sender] = 0;
        }
    }

    function deposit() public payable {
        credit[msg.sender] += msg.value;
        state.balance += msg.value;
    }
}