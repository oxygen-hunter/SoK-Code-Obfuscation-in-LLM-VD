pragma solidity ^0.4.16;

contract Ethraffle_v4b {
    struct Contestant {
        address addr;
        uint raffleId;
    }

    event RaffleResult(
        uint winningNumber,
        uint raffleId,
        address seed2,
        address seed1,
        address winningAddress,
        uint seed3,
        bytes32 randHash
    );

    event TicketPurchase(
        uint number,
        address contestant,
        uint raffleId
    );

    event TicketRefund(
        uint number,
        address contestant,
        uint raffleId
    );

    struct Constants {
        uint prize;
        uint fee;
        uint totalTickets;
        uint pricePerTicket;
    }

    Constants constants = Constants(2.5 ether, 0.03 ether, 50, (2.5 ether + 0.03 ether) / 50);
    address feeAddress;
    uint nextTicket = 0;
    uint[] gaps;
    mapping (uint => Contestant) contestants;

    struct Raffle {
        bool paused;
        uint raffleId;
        uint blockNumber;
    }

    Raffle raffle = Raffle(false, 1, block.number);

    function Ethraffle_v4b() public {
        feeAddress = msg.sender;
    }

    function () payable public {
        buyTickets();
    }

    function buyTickets() payable public {
        if (raffle.paused) {
            msg.sender.transfer(msg.value);
            return;
        }

        uint moneySent = msg.value;

        while (moneySent >= constants.pricePerTicket && nextTicket < constants.totalTickets) {
            uint currTicket = 0;
            if (gaps.length > 0) {
                currTicket = gaps[gaps.length-1];
                gaps.length--;
            } else {
                currTicket = nextTicket++;
            }

            contestants[currTicket] = Contestant(msg.sender, raffle.raffleId);
            TicketPurchase(currTicket, msg.sender, raffle.raffleId);
            moneySent -= constants.pricePerTicket;
        }

        if (nextTicket == constants.totalTickets) {
            chooseWinner();
        }

        if (moneySent > 0) {
            msg.sender.transfer(moneySent);
        }
    }

    function chooseWinner() private {
        address seed1 = contestants[uint(block.coinbase) % constants.totalTickets].addr;
        address seed2 = contestants[uint(msg.sender) % constants.totalTickets].addr;
        uint seed3 = block.difficulty;
        bytes32 randHash = keccak256(seed1, seed2, seed3);

        uint winningNumber = uint(randHash) % constants.totalTickets;
        address winningAddress = contestants[winningNumber].addr;
        RaffleResult(winningNumber, raffle.raffleId, seed2, seed1, winningAddress, seed3, randHash);

        raffle.raffleId++;
        nextTicket = 0;
        raffle.blockNumber = block.number;

        winningAddress.transfer(constants.prize);
        feeAddress.transfer(constants.fee);
    }

    function getRefund() public {
        uint refund = 0;
        for (uint i = 0; i < constants.totalTickets; i++) {
            if (msg.sender == contestants[i].addr && raffle.raffleId == contestants[i].raffleId) {
                refund += constants.pricePerTicket;
                contestants[i] = Contestant(address(0), 0);
                gaps.push(i);
                TicketRefund(i, msg.sender, raffle.raffleId);
            }
        }

        if (refund > 0) {
            msg.sender.transfer(refund);
        }
    }

    function endRaffle() public {
        if (msg.sender == feeAddress) {
            raffle.paused = true;

            for (uint i = 0; i < constants.totalTickets; i++) {
                if (raffle.raffleId == contestants[i].raffleId) {
                    TicketRefund(i, contestants[i].addr, raffle.raffleId);
                    contestants[i].addr.transfer(constants.pricePerTicket);
                }
            }

            RaffleResult(constants.totalTickets, raffle.raffleId, address(0), address(0), address(0), 0, 0);
            raffle.raffleId++;
            nextTicket = 0;
            raffle.blockNumber = block.number;
            gaps.length = 0;
        }
    }

    function togglePause() public {
        if (msg.sender == feeAddress) {
            raffle.paused = !raffle.paused;
        }
    }

    function kill() public {
        if (msg.sender == feeAddress) {
            selfdestruct(feeAddress);
        }
    }
}