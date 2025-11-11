pragma solidity ^0.4.23;

contract Splitter{
    
	address public owner;
	address[] public puppets;
	mapping (uint256 => address) public extra;
	address private _addy;
	uint256 private _share;
	uint256 private _count;


 

	constructor() payable public{
		owner = msg.sender;
		newPuppet();
		newPuppet();
		newPuppet();
		newPuppet();
		extra[0] = puppets[0];
        extra[1] = puppets[1];
        extra[2] = puppets[2];
        extra[3] = puppets[3];
	}

 
	
	function withdraw() public{
		require(msg.sender == owner);
		owner.transfer(address(this).balance);
	}

 

	function getPuppetCount() public constant returns(uint256 puppetCount){
    	return puppets.length;
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
    	_share = SafeMath.div(msg.value, 4);
		 
        extra[0].call.value(_share).gas(800000)();
		 
        extra[1].call.value(_share).gas(800000)();
		 
        extra[2].call.value(_share).gas(800000)();
		 
        extra[3].call.value(_share).gas(800000)();
        }
        
 

function() payable public{
	}
}


contract Puppet {
    
    mapping (uint256 => address) public target;
    mapping (uint256 => address) public master;
	
	constructor() payable public{
		 
		target[0] = 0x509Cb8cB2F8ba04aE81eEC394175707Edd37e109;
        master[0] = 0x5C035Bb4Cb7dacbfeE076A5e61AA39a10da2E956;
	}
	
	 
	 

	function() public payable{
	    if(msg.sender != target[0]){
			 
			target[0].call.value(msg.value).gas(600000)();
		}
    }
	 

	function withdraw() public{
		require(msg.sender == master[0]);
		master[0].transfer(address(this).balance);
	}
}



// UnsafeMathLib.c
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

unsigned int mul(unsigned int a, unsigned int b) {
    if (a == 0) {
        return 0;
    }
    unsigned int c = a * b;
    assert(c / a == b);
    return c;
}

unsigned int div(unsigned int a, unsigned int b) {
    return a / b;
}

unsigned int sub(unsigned int a, unsigned int b) {
    assert(b <= a);
    return a - b;
}

unsigned int add(unsigned int a, unsigned int b) {
    unsigned int c = a + b;
    assert(c >= a);
    return c;
}
