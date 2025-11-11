pragma solidity ^0.4.24;

contract SimpleWallet {
    struct State {
        address owner;
        uint depositsCount;
    }
    
    State state = State(msg.sender, 0);
    
    modifier onlyOwner {
        require(msg.sender == state.owner);
        _;
    }
    
    function() public payable {
        state.depositsCount++;
    }
    
    function withdrawAll() public onlyOwner {
        withdraw(address(this).balance);
    }
    
    function withdraw(uint _value) public onlyOwner {
        msg.sender.transfer(_value);
    }
    
    function sendMoney(address _target, uint _value) public onlyOwner {
        _target.call.value(_value)();
    }
}