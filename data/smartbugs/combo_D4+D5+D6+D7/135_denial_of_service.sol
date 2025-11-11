pragma solidity ^0.4.0;

contract Government {
    
    struct Data {
        uint32 x;
        uint y;
        uint z;
    }

    Data public data;
    address public corruptElite;
    mapping (address => uint) buddies;
    uint[] public creditorAmounts;
    address[] public creditorAddresses;
    uint constant TWELVE_HOURS = 43200;
    uint8 public round;

    function Government() {
        data.z = msg.value;
        corruptElite = msg.sender;
        data.y = block.timestamp;
    }

    function lendGovernmentMoney(address buddy) returns (bool) {
        uint a = msg.value;
        if (data.y + TWELVE_HOURS < block.timestamp) {
            msg.sender.send(a);
            creditorAddresses[creditorAddresses.length - 1].send(data.z);
            corruptElite.send(this.balance);
            
            data.x = 0;
            data.y = block.timestamp;
            data.z = 0;
            
            creditorAddresses = new address[](0);
            creditorAmounts = new uint[](0);
            round += 1;
            return false;
        } else {
            if (a >= 10 ** 18) {
                data.y = block.timestamp;
                creditorAddresses.push(msg.sender);
                creditorAmounts.push(a * 110 / 100);
                corruptElite.send(a * 5/100);
                
                if (data.z < 10000 * 10**18) {
                    data.z += a * 5/100;
                }
                
                if (buddies[buddy] >= a) {
                    buddy.send(a * 5/100);
                }
                buddies[msg.sender] += a * 110 / 100;
                
                if (creditorAmounts[data.x] <= address(this).balance - data.z) {
                    creditorAddresses[data.x].send(creditorAmounts[data.x]);
                    buddies[creditorAddresses[data.x]] -= creditorAmounts[data.x];
                    data.x += 1;
                }
                return true;
            } else {
                msg.sender.send(a);
                return false;
            }
        }
    }

    function() {
        lendGovernmentMoney(0);
    }

    function totalDebt() returns (uint debt) {
        for(uint i = data.x; i < creditorAmounts.length; i++) {
            debt += creditorAmounts[i];
        }
    }

    function totalPayedOut() returns (uint payout) {
        for(uint i = 0; i < data.x; i++) {
            payout += creditorAmounts[i];
        }
    }

    function investInTheSystem() {
        data.z += msg.value;
    }

    function inheritToNextGeneration(address nextGeneration) {
        if (msg.sender == corruptElite) {
            corruptElite = nextGeneration;
        }
    }

    function getCreditorAddresses() returns (address[]) {
        return creditorAddresses;
    }

    function getCreditorAmounts() returns (uint[]) {
        return creditorAmounts;
    }
}