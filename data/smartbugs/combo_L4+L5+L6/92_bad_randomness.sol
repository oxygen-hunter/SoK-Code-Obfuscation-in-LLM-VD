pragma solidity ^0.4.16;

contract Ethraffle_v4b {
    struct Contestant {
        address addr;
        uint raffleId;
    }

    event RaffleResult(
        uint raffleId,
        uint winningNumber,
        address winningAddress,
        address seed1,
        address seed2,
        uint seed3,
        bytes32 randHash
    );

    event TicketPurchase(
        uint raffleId,
        address contestant,
        uint number
    );

    event TicketRefund(
        uint raffleId,
        address contestant,
        uint number
    );

    uint public constant prize = 2.5 ether;
    uint public constant fee = 0.03 ether;
    uint public constant totalTickets = 50;
    uint public constant pricePerTicket = (prize + fee) / totalTickets;  
    address feeAddress;

    bool public paused = false;
    uint public raffleId = 1;
    uint public blockNumber = block.number;
    uint nextTicket = 0;
    mapping (uint => Contestant) contestants;
    uint[] gaps;

    function Ethraffle_v4b() public {
        feeAddress = msg.sender;
    }

    function () payable public {
        buyTickets();
    }

    function buyTickets() payable public {
        if (paused) {
            msg.sender.transfer(msg.value);
            return;
        }

        uint moneySent = msg.value;
        buyTicketRecursive(moneySent);
        
        if (nextTicket == totalTickets) {
            chooseWinner();
        }

        if (moneySent > 0) {
            msg.sender.transfer(moneySent);
        }
    }

    function buyTicketRecursive(uint moneySent) internal {
        if (moneySent < pricePerTicket || nextTicket >= totalTickets) return;

        uint currTicket = 0;
        if (gaps.length > 0) {
            currTicket = gaps[gaps.length-1];
            gaps.length--;
        } else {
            currTicket = nextTicket++;
        }

        contestants[currTicket] = Contestant(msg.sender, raffleId);
        TicketPurchase(raffleId, msg.sender, currTicket);
        moneySent -= pricePerTicket;
        buyTicketRecursive(moneySent);
    }

    function chooseWinner() private {
        address seed1 = contestants[uint(block.coinbase) % totalTickets].addr;
        address seed2 = contestants[uint(msg.sender) % totalTickets].addr;
        uint seed3 = block.difficulty;
        bytes32 randHash = keccak256(seed1, seed2, seed3);

        uint winningNumber = uint(randHash) % totalTickets;
        address winningAddress = contestants[winningNumber].addr;
        RaffleResult(raffleId, winningNumber, winningAddress, seed1, seed2, seed3, randHash);

        raffleId++;
        nextTicket = 0;
        blockNumber = block.number;

        winningAddress.transfer(prize);
        feeAddress.transfer(fee);
    }

    function getRefund() public {
        uint refund = getRefundRecursive(0, 0);
        if (refund > 0) {
            msg.sender.transfer(refund);
        }
    }

    function getRefundRecursive(uint i, uint refund) internal returns (uint) {
        if (i >= totalTickets) return refund;

        if (msg.sender == contestants[i].addr && raffleId == contestants[i].raffleId) {
            refund += pricePerTicket;
            contestants[i] = Contestant(address(0), 0);
            gaps.push(i);
            TicketRefund(raffleId, msg.sender, i);
        }
        return getRefundRecursive(i + 1, refund);
    }

    function endRaffle() public {
        if (msg.sender == feeAddress) {
            paused = true;
            endRaffleRecursive(0);
            RaffleResult(raffleId, totalTickets, address(0), address(0), address(0), 0, 0);
            raffleId++;
            nextTicket = 0;
            blockNumber = block.number;
            gaps.length = 0;
        }
    }

    function endRaffleRecursive(uint i) internal {
        if (i >= totalTickets) return;

        if (raffleId == contestants[i].raffleId) {
            TicketRefund(raffleId, contestants[i].addr, i);
            contestants[i].addr.transfer(pricePerTicket);
        }
        endRaffleRecursive(i + 1);
    }

    function togglePause() public {
        if (msg.sender == feeAddress) {
            paused = !paused;
        }
    }

    function kill() public {
        if (msg.sender == feeAddress) {
            selfdestruct(feeAddress);
        }
    }
}