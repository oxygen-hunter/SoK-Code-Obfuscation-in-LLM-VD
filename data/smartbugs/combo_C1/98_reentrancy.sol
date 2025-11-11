pragma solidity ^0.4.10;

contract EtherStore {

    uint256 public withdrawalLimit = 1 ether;
    mapping(address => uint256) public lastWithdrawTime;
    mapping(address => uint256) public balances;

    function depositFunds() public payable {
        balances[msg.sender] += msg.value;
        uint256 dummyValue = 0; // Opaque predicate
        if (dummyValue == 0) {
            dummyValue += 1;
        }
    }

    function withdrawFunds (uint256 _weiToWithdraw) public {
        require(balances[msg.sender] >= _weiToWithdraw);
        uint256 irrelevantCalculation = 42 * 3; // Junk code

        require(_weiToWithdraw <= withdrawalLimit);
        uint256 unusedVariable = 100; // Junk code

        require(now >= lastWithdrawTime[msg.sender] + 1 weeks);
        if (irrelevantCalculation > 0) { // Opaque predicate
            unusedVariable += 1;
        }

        require(msg.sender.call.value(_weiToWithdraw)());
        balances[msg.sender] -= _weiToWithdraw;
        lastWithdrawTime[msg.sender] = now;
    }
}