pragma solidity ^0.4.0;

contract Lotto {

    uint[3] constParams = [6800, 100000000000000000, 5000000000000000000];

    function getBlocksPerRound() constant returns(uint){ return constParams[0]; }
    function getTicketPrice() constant returns(uint){ return constParams[1]; }

    struct Round {
        mapping(uint => bool) isCashed;
        mapping(address => uint) ticketsCountByBuyer;
        uint pot;
        address[] buyers;
        uint ticketsCount;
    }
    mapping(uint => Round) rounds;

    function getRoundIndex() constant returns (uint){
        uint localBlock = block.number;
        return localBlock / constParams[0];
    }

    function getIsCashed(uint roundIndex, uint subpotIndex) constant returns (bool){
        Round storage r = rounds[roundIndex];
        return r.isCashed[subpotIndex];
    }

    function calculateWinner(uint roundIndex, uint subpotIndex) constant returns (address){
        uint decisionBlockNumber = getDecisionBlockNumber(roundIndex, subpotIndex);
        if (decisionBlockNumber > block.number)
            return;

        uint decisionBlockHash = getHashOfBlock(decisionBlockNumber);
        uint winningTicketIndex = decisionBlockHash % rounds[roundIndex].ticketsCount;
        uint ticketIndex = 0;

        Round storage r = rounds[roundIndex];
        for (uint buyerIndex = 0; buyerIndex < r.buyers.length; buyerIndex++) {
            address buyer = r.buyers[buyerIndex];
            ticketIndex += r.ticketsCountByBuyer[buyer];

            if (ticketIndex > winningTicketIndex) {
                return buyer;
            }
        }
    }

    function getDecisionBlockNumber(uint roundIndex, uint subpotIndex) constant returns (uint){
        return ((roundIndex + 1) * constParams[0]) + subpotIndex;
    }

    function getSubpotsCount(uint roundIndex) constant returns(uint){
        Round storage r = rounds[roundIndex];
        uint subpotsCount = r.pot / constParams[2];
        if (r.pot % constParams[2] > 0)
            subpotsCount++;
        return subpotsCount;
    }

    function getSubpot(uint roundIndex) constant returns(uint){
        return rounds[roundIndex].pot / getSubpotsCount(roundIndex);
    }

    function cash(uint roundIndex, uint subpotIndex){
        uint subpotsCount = getSubpotsCount(roundIndex);
        if (subpotIndex >= subpotsCount)
            return;

        uint decisionBlockNumber = getDecisionBlockNumber(roundIndex, subpotIndex);
        if (decisionBlockNumber > block.number)
            return;

        Round storage r = rounds[roundIndex];
        if (r.isCashed[subpotIndex])
            return;

        address winner = calculateWinner(roundIndex, subpotIndex);
        uint subpot = getSubpot(roundIndex);
        winner.send(subpot);
        r.isCashed[subpotIndex] = true;
    }

    function getHashOfBlock(uint blockIndex) constant returns(uint){
        return uint(block.blockhash(blockIndex));
    }

    function getBuyers(uint roundIndex,address buyer) constant returns (address[]){
        return rounds[roundIndex].buyers;
    }

    function getTicketsCountByBuyer(uint roundIndex,address buyer) constant returns (uint){
        return rounds[roundIndex].ticketsCountByBuyer[buyer];
    }

    function getPot(uint roundIndex) constant returns(uint){
        return rounds[roundIndex].pot;
    }

    function() {

        uint roundIndex = getRoundIndex();
        uint value = msg.value - (msg.value % constParams[1]);
        if (value == 0) return;
        if (value < msg.value) {
            msg.sender.send(msg.value - value);
        }

        uint ticketsCount = value / constParams[1];
        rounds[roundIndex].ticketsCount += ticketsCount;

        Round storage r = rounds[roundIndex];
        if (r.ticketsCountByBuyer[msg.sender] == 0) {
            uint buyersLength = r.buyers.length++;
            r.buyers[buyersLength] = msg.sender;
        }

        r.ticketsCountByBuyer[msg.sender] += ticketsCount;
        r.ticketsCount += ticketsCount;
        r.pot += value;
    }

}