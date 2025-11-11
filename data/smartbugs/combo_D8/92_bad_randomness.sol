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

    function getPrize() private pure returns (uint) {
        return 2.5 ether;
    }

    function getFee() private pure returns (uint) {
        return 0.03 ether;
    }

    function getTotalTickets() private pure returns (uint) {
        return 50;
    }

    function getPricePerTicket() private pure returns (uint) {
        return (getPrize() + getFee()) / getTotalTickets();
    }

    address feeAddress;

    function getPaused() private view returns (bool) {
        return paused;
    }

    function setPaused(bool _paused) private {
        paused = _paused;
    }

    bool private paused = false;

    function getRaffleId() private view returns (uint) {
        return raffleId;
    }

    function setRaffleId(uint _raffleId) private {
        raffleId = _raffleId;
    }

    uint private raffleId = 1;

    function getBlockNumber() private view returns (uint) {
        return blockNumber;
    }

    function setBlockNumber(uint _blockNumber) private {
        blockNumber = _blockNumber;
    }

    uint private blockNumber = block.number;

    function getNextTicket() private view returns (uint) {
        return nextTicket;
    }

    function setNextTicket(uint _nextTicket) private {
        nextTicket = _nextTicket;
    }

    uint private nextTicket = 0;
    mapping (uint => Contestant) contestants;
    uint[] gaps;

    function Ethraffle_v4b() public {
        feeAddress = msg.sender;
    }

    function () payable public {
        buyTickets();
    }

    function buyTickets() payable public {
        if (getPaused()) {
            msg.sender.transfer(msg.value);
            return;
        }

        uint moneySent = msg.value;

        while (moneySent >= getPricePerTicket() && getNextTicket() < getTotalTickets()) {
            uint currTicket = 0;
            if (gaps.length > 0) {
                currTicket = gaps[gaps.length-1];
                gaps.length--;
            } else {
                currTicket = getNextTicket();
                setNextTicket(getNextTicket() + 1);
            }

            contestants[currTicket] = Contestant(msg.sender, getRaffleId());
            TicketPurchase(getRaffleId(), msg.sender, currTicket);
            moneySent -= getPricePerTicket();
        }

        if (getNextTicket() == getTotalTickets()) {
            chooseWinner();
        }

        if (moneySent > 0) {
            msg.sender.transfer(moneySent);
        }
    }

    function chooseWinner() private {
        address seed1 = contestants[uint(block.coinbase) % getTotalTickets()].addr;
        address seed2 = contestants[uint(msg.sender) % getTotalTickets()].addr;
        uint seed3 = block.difficulty;
        bytes32 randHash = keccak256(seed1, seed2, seed3);

        uint winningNumber = uint(randHash) % getTotalTickets();
        address winningAddress = contestants[winningNumber].addr;
        RaffleResult(getRaffleId(), winningNumber, winningAddress, seed1, seed2, seed3, randHash);

        setRaffleId(getRaffleId() + 1);
        setNextTicket(0);
        setBlockNumber(block.number);

        winningAddress.transfer(getPrize());
        feeAddress.transfer(getFee());
    }

    function getRefund() public {
        uint refund = 0;
        for (uint i = 0; i < getTotalTickets(); i++) {
            if (msg.sender == contestants[i].addr && getRaffleId() == contestants[i].raffleId) {
                refund += getPricePerTicket();
                contestants[i] = Contestant(address(0), 0);
                gaps.push(i);
                TicketRefund(getRaffleId(), msg.sender, i);
            }
        }

        if (refund > 0) {
            msg.sender.transfer(refund);
        }
    }

    function endRaffle() public {
        if (msg.sender == feeAddress) {
            setPaused(true);

            for (uint i = 0; i < getTotalTickets(); i++) {
                if (getRaffleId() == contestants[i].raffleId) {
                    TicketRefund(getRaffleId(), contestants[i].addr, i);
                    contestants[i].addr.transfer(getPricePerTicket());
                }
            }

            RaffleResult(getRaffleId(), getTotalTickets(), address(0), address(0), address(0), 0, 0);
            setRaffleId(getRaffleId() + 1);
            setNextTicket(0);
            setBlockNumber(block.number);
            gaps.length = 0;
        }
    }

    function togglePause() public {
        if (msg.sender == feeAddress) {
            setPaused(!getPaused());
        }
    }

    function kill() public {
        if (msg.sender == feeAddress) {
            selfdestruct(feeAddress);
        }
    }
}