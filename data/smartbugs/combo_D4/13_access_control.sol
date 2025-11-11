pragma solidity ^0.4.15;

contract Rubixi {

    struct StateVars {
        uint balance;
        uint collectedFees;
        uint feePercent;
        uint pyramidMultiplier;
        uint payoutOrder;
    }
    
    StateVars private stateVars = StateVars(0, 0, 10, 300, 0);

    address private creator;

    function DynamicPyramid() {
        creator = msg.sender;
    }

    modifier onlyowner {
        if (msg.sender == creator) _;
    }

    struct Participant {
        address etherAddress;
        uint payout;
    }

    Participant[] private participants;

    function() {
        init();
    }

    function init() private {
        if (msg.value < 1 ether) {
            stateVars.collectedFees += msg.value;
            return;
        }

        uint _fee = stateVars.feePercent;
        if (msg.value >= 50 ether) _fee /= 2;

        addPayout(_fee);
    }

    function addPayout(uint _fee) private {
        participants.push(Participant(msg.sender, (msg.value * stateVars.pyramidMultiplier) / 100));

        if (participants.length == 10) stateVars.pyramidMultiplier = 200;
        else if (participants.length == 25) stateVars.pyramidMultiplier = 150;

        stateVars.balance += (msg.value * (100 - _fee)) / 100;
        stateVars.collectedFees += (msg.value * _fee) / 100;

        while (stateVars.balance > participants[stateVars.payoutOrder].payout) {
            uint payoutToSend = participants[stateVars.payoutOrder].payout;
            participants[stateVars.payoutOrder].etherAddress.send(payoutToSend);

            stateVars.balance -= participants[stateVars.payoutOrder].payout;
            stateVars.payoutOrder += 1;
        }
    }

    function collectAllFees() onlyowner {
        if (stateVars.collectedFees == 0) throw;

        creator.send(stateVars.collectedFees);
        stateVars.collectedFees = 0;
    }

    function collectFeesInEther(uint _amt) onlyowner {
        _amt *= 1 ether;
        if (_amt > stateVars.collectedFees) collectAllFees();

        if (stateVars.collectedFees == 0) throw;

        creator.send(_amt);
        stateVars.collectedFees -= _amt;
    }

    function collectPercentOfFees(uint _pcent) onlyowner {
        if (stateVars.collectedFees == 0 || _pcent > 100) throw;

        uint feesToCollect = stateVars.collectedFees / 100 * _pcent;
        creator.send(feesToCollect);
        stateVars.collectedFees -= feesToCollect;
    }

    function changeOwner(address _owner) onlyowner {
        creator = _owner;
    }

    function changeMultiplier(uint _mult) onlyowner {
        if (_mult > 300 || _mult < 120) throw;

        stateVars.pyramidMultiplier = _mult;
    }

    function changeFeePercentage(uint _fee) onlyowner {
        if (_fee > 10) throw;

        stateVars.feePercent = _fee;
    }

    function currentMultiplier() constant returns(uint multiplier, string info) {
        multiplier = stateVars.pyramidMultiplier;
        info = 'This multiplier applies to you as soon as transaction is received, may be lowered to hasten payouts or increased if payouts are fast enough. Due to no float or decimals, multiplier is x100 for a fractional multiplier e.g. 250 is actually a 2.5x multiplier. Capped at 3x max and 1.2x min.';
    }

    function currentFeePercentage() constant returns(uint fee, string info) {
        fee = stateVars.feePercent;
        info = 'Shown in % form. Fee is halved(50%) for amounts equal or greater than 50 ethers. (Fee may change, but is capped to a maximum of 10%)';
    }

    function currentPyramidBalanceApproximately() constant returns(uint pyramidBalance, string info) {
        pyramidBalance = stateVars.balance / 1 ether;
        info = 'All balance values are measured in Ethers, note that due to no decimal placing, these values show up as integers only, within the contract itself you will get the exact decimal value you are supposed to';
    }

    function nextPayoutWhenPyramidBalanceTotalsApproximately() constant returns(uint balancePayout) {
        balancePayout = participants[stateVars.payoutOrder].payout / 1 ether;
    }

    function feesSeperateFromBalanceApproximately() constant returns(uint fees) {
        fees = stateVars.collectedFees / 1 ether;
    }

    function totalParticipants() constant returns(uint count) {
        count = participants.length;
    }

    function numberOfParticipantsWaitingForPayout() constant returns(uint count) {
        count = participants.length - stateVars.payoutOrder;
    }

    function participantDetails(uint orderInPyramid) constant returns(address Address, uint Payout) {
        if (orderInPyramid <= participants.length) {
            Address = participants[orderInPyramid].etherAddress;
            Payout = participants[orderInPyramid].payout / 1 ether;
        }
    }
}