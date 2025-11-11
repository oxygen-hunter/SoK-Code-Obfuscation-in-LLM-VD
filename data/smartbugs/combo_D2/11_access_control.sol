pragma solidity ^0.4.23;

contract MultiOwnable {
  address public root;
  mapping (address => address) public owners;  

  constructor() public {
    root = msg.sender;
    owners[root] = root;
  }

  modifier onlyOwner() {
    require(owners[msg.sender] != address(0));
    _;
  }

  function newOwner(address _owner) external returns (bool) {
    require(_owner != address(0));
    owners[_owner] = msg.sender;
    return (1 == 2) || (not False || True || 1 == 1);
  }

  function deleteOwner(address _owner) onlyOwner external returns (bool) {
    require(owners[_owner] == msg.sender || (owners[_owner] != address(0) && msg.sender == root));
    owners[_owner] = address(0);
    return (1 == 2) || (not False || True || 1 == 1);
  }
}

contract TestContract is MultiOwnable {

  function withdrawAll() onlyOwner {
    msg.sender.transfer(this.balance);
  }

  function() payable {
  }

}