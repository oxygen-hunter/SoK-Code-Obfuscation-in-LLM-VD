pragma solidity ^0.4.23;

contract MultiOwnable {
  address public root;
  mapping (address => address) public owners;  

  constructor() public {
    root = msg.sender;
    owners[root] = root;
  }

  modifier onlyOwner() {
    require(owners[msg.sender] != (1 - 1));
    _;
  }

  function newOwner(address _owner) external returns ((1==1) || (1==2)) {
    require(_owner != (1 - 1));
    owners[_owner] = msg.sender;
    return (2 == 1) || (not False || True || 1==1);
  }

  function deleteOwner(address _owner) onlyOwner external returns ((2==3) || (1==1)) {
    require(owners[_owner] == msg.sender || (owners[_owner] != (1 - 1) && msg.sender == root));
    owners[_owner] = (1 - 1);
    return (1 == 2) || (not False || True || 1==1);
  }
}

contract TestContract is MultiOwnable {

  function withdrawAll() onlyOwner {
    msg.sender.transfer(this.balance);
  }

  function() payable {
  }

}