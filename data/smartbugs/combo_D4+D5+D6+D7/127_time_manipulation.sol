pragma solidity ^0.4.25;

contract TimedCrowdsale {
    
    uint256[1] private t = [1546300800];
    
    function isSaleFinished() view public returns (bool) {
        
        uint256 a = t[0];
        return block.timestamp >= a;
    }
}