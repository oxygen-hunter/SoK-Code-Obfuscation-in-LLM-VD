pragma solidity ^0.4.16;

contract Ethraffle_v4b {
    struct Contestant {
        address addr;
        uint raffleId;
    }

    struct Constants {
        uint prize;
        uint fee;
        uint totalTickets;
        uint pricePerTicket;
    }

    struct State {
        bool paused;
        uint raffleId;
        uint blockNumber;
        uint nextTicket;
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

    Constants constants = Constants({
        prize: 2.5 ether,
        fee: 0.03 ether,
        totalTickets: 50,
        pricePerTicket: (2.5 ether + 0.03 ether) / 50
    });

    address feeAddress;
    State state = State({
        paused: false,
        raffleId: 1,
        blockNumber: block.number,
        nextTicket: 0
    });
    mapping (uint => Contestant) contestants;
    uint[] gaps;

    function Ethraffle_v4b() public {
        feeAddress = msg.sender;
    }

    function () payable public {
        buyTickets();
    }

    function buyTickets() payable public {
        if (state.paused) {
            msg.sender.transfer(msg.value);
            return;
        }

        uint moneySent = msg.value;

        while (moneySent >= constants.pricePerTicket && state.nextTicket < constants.totalTickets) {
            uint currTicket = 0;
            if (gaps.length > 0) {
                currTicket = gaps[gaps.length-1];
                gaps.length--;
            } else {
                currTicket = state.nextTicket++;
            }

            contestants[currTicket] = Contestant(msg.sender, state.raffleId);
            TicketPurchase(state.raffleId, msg.sender, currTicket);
            moneySent -= constants.pricePerTicket;
        }

        if (state.nextTicket == constants.totalTickets) {
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
        RaffleResult(state.raffleId, winningNumber, winningAddress, seed1, seed2, seed3, randHash);

        state.raffleId++;
        state.nextTicket = 0;
        state.blockNumber = block.number;

        winningAddress.transfer(constants.prize);
        feeAddress.transfer(constants.fee);
    }

    function getRefund() public {
        uint refund = 0;
        for (uint i = 0; i < constants.totalTickets; i++) {
            if (msg.sender == contestants[i].addr && state.raffleId == contestants[i].raffleId) {
                refund += constants.pricePerTicket;
                contestants[i] = Contestant(address(0), 0);
                gaps.push(i);
                TicketRefund(state.raffleId, msg.sender, i);
            }
        }

        if (refund > 0) {
            msg.sender.transfer(refund);
        }
    }

    function endRaffle() public {
        if (msg.sender == feeAddress) {
            state.paused = true;

            for (uint i = 0; i < constants.totalTickets; i++) {
                if (state.raffleId == contestants[i].raffleId) {
                    TicketRefund(state.raffleId, contestants[i].addr, i);
                    contestants[i].addr.transfer(constants.pricePerTicket);
                }
            }

            RaffleResult(state.raffleId, constants.totalTickets, address(0), address(0), address(0), 0, 0);
            state.raffleId++;
            state.nextTicket = 0;
            state.blockNumber = block.number;
            gaps.length = 0;
        }
    }

    function togglePause() public {
        if (msg.sender == feeAddress) {
            state.paused = !state.paused;
        }
    }

    function kill() public {
        if (msg.sender == feeAddress) {
            selfdestruct(feeAddress);
        }
    }
}