pragma solidity ^0.4.0;

contract Lotto {

    uint constant public blocksPerRound = 6800;
    uint constant public ticketPrice = 100000000000000000;
    uint constant public blockReward = 5000000000000000000;

    mapping(uint => struct {
        address[] buyers;
        uint pot;
        uint ticketsCount;
        mapping(uint=>bool) isCashed;
        mapping(address=>uint) ticketsCountByBuyer;
    }) rounds;

    function getBlocksPerRound() constant returns(uint){ return blocksPerRound; }
    function getTicketPrice() constant returns(uint){ return ticketPrice; }

    function getRoundIndex() constant returns (uint){
        return block.number/blocksPerRound;
    }

    function getIsCashed(uint roundIndex,uint subpotIndex) constant returns (bool){
        return rounds[roundIndex].isCashed[subpotIndex];
    }

    function calculateWinner(uint roundIndex, uint subpotIndex) constant returns(address){
        uint decisionBlockNumber = getDecisionBlockNumber(roundIndex,subpotIndex);

        if(decisionBlockNumber>block.number)
            return;

        uint decisionBlockHash = getHashOfBlock(decisionBlockNumber);
        uint winningTicketIndex = decisionBlockHash%rounds[roundIndex].ticketsCount;

        uint ticketIndex = uint256(0);

        for(uint buyerIndex = 0; buyerIndex<rounds[roundIndex].buyers.length; buyerIndex++){
            address buyer = rounds[roundIndex].buyers[buyerIndex];
            ticketIndex+=rounds[roundIndex].ticketsCountByBuyer[buyer];

            if(ticketIndex>winningTicketIndex){
                return buyer;
            }
        }
    }

    function getDecisionBlockNumber(uint roundIndex,uint subpotIndex) constant returns (uint){
        return ((roundIndex+1)*blocksPerRound)+subpotIndex;
    }

    function getSubpotsCount(uint roundIndex) constant returns(uint){
        uint subpotsCount = rounds[roundIndex].pot/blockReward;

        if(rounds[roundIndex].pot%blockReward>0)
            subpotsCount++;

        return subpotsCount;
    }

    function getSubpot(uint roundIndex) constant returns(uint){
        return rounds[roundIndex].pot/getSubpotsCount(roundIndex);
    }

    function cash(uint roundIndex, uint subpotIndex){

        uint subpotsCount = getSubpotsCount(roundIndex);

        if(subpotIndex>=subpotsCount)
            return;

        uint decisionBlockNumber = getDecisionBlockNumber(roundIndex,subpotIndex);

        if(decisionBlockNumber>block.number)
            return;

        if(rounds[roundIndex].isCashed[subpotIndex])
            return;

        address winner = calculateWinner(roundIndex,subpotIndex);
        uint subpot = getSubpot(roundIndex);

        winner.send(subpot);
        rounds[roundIndex].isCashed[subpotIndex] = true;
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
        uint value = msg.value-(msg.value%ticketPrice);

        if(value==0) return;

        if(value<msg.value){
            msg.sender.send(msg.value-value);
        }

        uint ticketsCount = value/ticketPrice;
        rounds[roundIndex].ticketsCount+=ticketsCount;

        if(rounds[roundIndex].ticketsCountByBuyer[msg.sender]==0){
            uint buyersLength = rounds[roundIndex].buyers.length++;
            rounds[roundIndex].buyers[buyersLength] = msg.sender;
        }

        rounds[roundIndex].ticketsCountByBuyer[msg.sender]+=ticketsCount;
        rounds[roundIndex].ticketsCount+=ticketsCount;
        rounds[roundIndex].pot+=value;
    }

}