// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2015 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "pow.h"

#include "arith_uint256.h"
#include "chain.h"
#include "primitives/block.h"
#include "uint256.h"
#include "util.h"
#include <stdio.h>

static arith_uint256 GetTargetLimit(int64_t nTime, const Consensus::Params& params, bool fProofOfStake)
{
    uint256 nLimit;

    if (fProofOfStake) {
        if (params.IsProtocolV3(nTime))
            nLimit = params.posLimitV2;
        else
            nLimit = params.posLimit;
    } else {
        nLimit = params.powLimit;
    }

    return UintToArith256(nLimit);
}

unsigned int static GetNextWorkRequired_V1(const CBlockIndex* pindexLast, const Consensus::Params& params, const CBlockHeader *pblock)
{
    const arith_uint256 bnPowLimit = UintToArith256(params.powLimit);
    int64_t retargetTimespan = params.nTargetTimespan;
    int64_t retargetSpacing = params.nTargetSpacing;
    int64_t retargetInterval = retargetTimespan / retargetSpacing;

    // Only change once per interval
    if ((pindexLast->nHeight + 1) % retargetInterval != 0)
    {
        // Special difficulty rule for testnet:
        if (params.fPowAllowMinDifficultyBlocks)
        {
            // If the new block's timestamp is more than 2* nTargetSpacing minutes
            // then allow mining of a min-difficulty block.
            if (pblock->GetBlockTime() > pindexLast->GetBlockTime() + retargetSpacing*2)
                return bnPowLimit.GetCompact();
            else
            {
                // Return the last non-special-min-difficulty-rules-block
                const CBlockIndex* pindex = pindexLast;
                while (pindex->pprev && pindex->nHeight % retargetInterval != 0 && pindex->nBits == bnPowLimit.GetCompact())
                    pindex = pindex->pprev;
                return pindex->nBits;
            }
        }
        return pindexLast->nBits;
    }

    // Potcoin: This fixes an issue where a 51% attack can change difficulty at will.
    // Go back the full period unless it's the first retarget after genesis. Code courtesy of Art Forz
    int blockstogoback = retargetInterval-1;
    if ((pindexLast->nHeight + 1) != retargetInterval)
    blockstogoback = retargetInterval;

    // Go back by what we want to be 14 days worth of blocks
    const CBlockIndex* pindexFirst = pindexLast;
    for (int i = 0; pindexFirst && i < blockstogoback; i++)
        pindexFirst = pindexFirst->pprev;
    assert(pindexFirst);

    // Limit adjustment step
    const int64_t nActualTimespan = pindexLast->GetBlockTime() - pindexFirst->GetBlockTime();
    int64_t nModulatedTimespan = nActualTimespan;

    if (nModulatedTimespan < retargetTimespan / 4)
    nModulatedTimespan = retargetTimespan / 4;
    if (nModulatedTimespan > retargetTimespan*4)
    nModulatedTimespan = retargetTimespan*4;

    // Retarget
    arith_uint256 bnNew;
    bnNew.SetCompact(pindexLast->nBits);
    bnNew *= nModulatedTimespan;
    bnNew /= retargetTimespan;

    if (bnNew > bnPowLimit)
        bnNew = bnPowLimit;

    return bnNew.GetCompact();
}

unsigned int static KimotoGravityWell(const CBlockIndex* pindexLast, const Consensus::Params& params, const CBlockHeader *pblock) {
    // Difficulty formula, Megacoin - Kimoto Gravity Well
    const CBlockIndex *BlockLastSolved = pindexLast;
    const CBlockIndex *BlockReading = pindexLast;

    uint64_t PastBlocksMass = 0;
    int64_t PastRateActualSeconds = 0;
    int64_t PastRateTargetSeconds = 0;
    double PastRateAdjustmentRatio = double(1);
    arith_uint256 PastDifficultyAverage;
    arith_uint256 PastDifficultyAveragePrev;
    double EventHorizonDeviation;
    double EventHorizonDeviationFast;
    double EventHorizonDeviationSlow;

    static const int64_t nTargetSpacing = params.nTargetSpacing;
    static const unsigned int TimeDaySeconds = 60 * 60 * 24;
    int64_t PastSecondsMin = TimeDaySeconds * 0.01;
    int64_t PastSecondsMax = TimeDaySeconds * 0.14;
    uint64_t PastBlocksMin = PastSecondsMin / nTargetSpacing;
    uint64_t PastBlocksMax = PastSecondsMax / nTargetSpacing;
	const arith_uint256 bnPowLimit = UintToArith256(params.powLimit);
	
	if (BlockLastSolved == NULL || BlockLastSolved->nHeight == 0 || (uint64_t)BlockLastSolved->nHeight < PastBlocksMin) { return bnPowLimit.GetCompact(); }

    int64_t LatestBlockTime = BlockLastSolved->GetBlockTime();

    for (unsigned int i = 1; BlockReading && BlockReading->nHeight > 0; i++) {
        if (PastBlocksMax > 0 && i > PastBlocksMax) {
            break;
        }
        PastBlocksMass++;

        PastDifficultyAverage.SetCompact(BlockReading->nBits);
        if (i > 1) {
            if (PastDifficultyAverage >= PastDifficultyAveragePrev)
                PastDifficultyAverage = ((PastDifficultyAverage - PastDifficultyAveragePrev) / i) + PastDifficultyAveragePrev;
            else
                PastDifficultyAverage = PastDifficultyAveragePrev - ((PastDifficultyAveragePrev - PastDifficultyAverage) / i);
        }
        PastDifficultyAveragePrev = PastDifficultyAverage;

        if (LatestBlockTime < BlockReading->GetBlockTime()) {
            if (BlockReading->nHeight > 158000) { // Hard fork block number
                LatestBlockTime = BlockReading->GetBlockTime();
            }
        }

        PastRateActualSeconds = LatestBlockTime - BlockReading->GetBlockTime();
        PastRateTargetSeconds = nTargetSpacing * PastBlocksMass;
        PastRateAdjustmentRatio = double(1);

        if (BlockReading->nHeight > 158000) { // Hard fork block number
            if (PastRateActualSeconds < 1) {
                PastRateActualSeconds = 1;
            }
        } else {
            if (PastRateActualSeconds < 0) {
                PastRateActualSeconds = 0;
            }
        }

        if (PastRateActualSeconds != 0 && PastRateTargetSeconds != 0) {
            PastRateAdjustmentRatio = double(PastRateTargetSeconds) / double(PastRateActualSeconds);
        }
        EventHorizonDeviation = 1 + (0.7084 * std::pow((double(PastBlocksMass)/double(144)), -1.228));
        EventHorizonDeviationFast = EventHorizonDeviation;
        EventHorizonDeviationSlow  = 1 / EventHorizonDeviation;

        if (PastBlocksMass >= PastBlocksMin) {
            if ((PastRateAdjustmentRatio <= EventHorizonDeviationSlow) || (PastRateAdjustmentRatio >= EventHorizonDeviationFast)) {
                assert(BlockReading);
                break;
            }
        }
        if (BlockReading->pprev == NULL) {
            assert(BlockReading);
            break;
        }
        BlockReading = BlockReading->pprev;
    }

    arith_uint256 bnNew(PastDifficultyAverage);
    if (PastRateActualSeconds != 0 && PastRateTargetSeconds != 0) {
            bnNew *= PastRateActualSeconds;
            bnNew /= PastRateTargetSeconds;
	    }
		if (bnNew > bnPowLimit) {
		    bnNew = bnPowLimit;
        }

    return bnNew.GetCompact();
}

unsigned int static GetNextWorkRequired_V2(const CBlockIndex* pindexLast, const Consensus::Params& params, const CBlockHeader *pblock) {
    return KimotoGravityWell(pindexLast, params, pblock);
}

unsigned int static GetNextWorkRequired_V3(const CBlockIndex* pindexLast, const Consensus::Params& params, const CBlockHeader *pblock) {
    const arith_uint256 bnPowLimit = UintToArith256(params.powLimit);
    int64_t retargetTimespan = params.nTargetTimespanNEW;
    int64_t retargetSpacing = params.nTargetSpacing;
    int64_t retargetInterval = retargetTimespan / retargetSpacing;

    // Only change once per interval
    if ((pindexLast->nHeight + 1) % retargetInterval != 0)
    {
        // Special difficulty rule for testnet:
        if (params.fPowAllowMinDifficultyBlocks)
        {
            // If the new block's timestamp is more than 2* nTargetSpacing minutes
            // then allow mining of a min-difficulty block.
            if (pblock->GetBlockTime() > pindexLast->GetBlockTime() + retargetSpacing*2)
                return bnPowLimit.GetCompact();
            else
            {
                // Return the last non-special-min-difficulty-rules-block
                const CBlockIndex* pindex = pindexLast;
                while (pindex->pprev && pindex->nHeight % retargetInterval != 0 && pindex->nBits == bnPowLimit.GetCompact())
                    pindex = pindex->pprev;
                return pindex->nBits;
            }
        }
        return pindexLast->nBits;
    }
    // Litecoin: This fixes an issue where a 51% attack can change difficulty at will.
    // Go back the full period unless it's the first retarget after genesis. Code courtesy of Art Forz
    int blockstogoback = retargetInterval-1;
    if ((pindexLast->nHeight + 1) != retargetInterval)
    blockstogoback = retargetInterval;

    // Go back by what we want to be 14 days worth of blocks
    int nHeightFirst = pindexLast->nHeight - blockstogoback;
    assert(nHeightFirst >= 0);
    const CBlockIndex* pindexFirst = pindexLast->GetAncestor(nHeightFirst);
    assert(pindexFirst);

    const int64_t nActualTimespan = pindexLast->GetBlockTime() - pindexFirst->GetBlockTime();
    int64_t nModulatedTimespan = nActualTimespan;

    // DigiShield implementation - thanks to RealSolid & WDC for this code
    // amplitude filter - thanks to daft27 for this code
    nModulatedTimespan = retargetTimespan + (nModulatedTimespan - retargetTimespan) / 8;
    if (nModulatedTimespan < (retargetTimespan - (retargetTimespan / 4)))
        nModulatedTimespan = (retargetTimespan - (retargetTimespan / 4));
    if (nModulatedTimespan > (retargetTimespan + (retargetTimespan / 2)))
        nModulatedTimespan = (retargetTimespan + (retargetTimespan/2));

    // Retarget
    arith_uint256 bnNew;
    bnNew.SetCompact(pindexLast->nBits);
    bnNew *= nModulatedTimespan;
    bnNew /= retargetTimespan;

    if (bnNew > bnPowLimit)
        bnNew = bnPowLimit;

    return bnNew.GetCompact();
}

unsigned int static GetPoSV1Difficulty(const CBlockIndex* pindexLast, const Consensus::Params& params, const CBlockHeader *pblock)
{
	const arith_uint256 bnTargetLimit = UintToArith256(params.powLimit);
	const CBlockIndex* pindexPrev = GetLastBlockIndex(pindexLast, true);
    const CBlockIndex* pindexPrevPrev = GetLastBlockIndex(pindexPrev->pprev, true);

	// Reset difficulty for PoS switchover
	if (pindexLast->nHeight < params.nLastPOWBlock + 50)
		return bnTargetLimit.GetCompact();

    int64_t nTargetSpacing = params.nTargetSpacing;
    int64_t nActualSpacing = pindexPrev->GetBlockTime() - pindexPrevPrev->GetBlockTime();

	 // Normalize extreme values
	 if (nActualSpacing < 1)
	 	nActualSpacing = 1;
	 if (nActualSpacing > 2200)
	 	nActualSpacing = 2200;

    // ppcoin: target change every block
    // ppcoin: retarget with exponential moving toward target spacing
    arith_uint256 bnNew;
    bnNew.SetCompact(pindexPrev->nBits);
    int64_t nInterval = params.nTargetTimespan / nTargetSpacing;
    bnNew *= ((nInterval - 1) * nTargetSpacing + nActualSpacing + nActualSpacing);
    bnNew /= ((nInterval + 1) * nTargetSpacing);

    if (bnNew > bnTargetLimit)
        bnNew = bnTargetLimit;

    return bnNew.GetCompact();
}

unsigned int CalculateNextTargetRequired(const CBlockIndex* pindexLast, int64_t nFirstBlockTime, const Consensus::Params& params, bool fProofOfStake)
{
    int64_t nTargetSpacing = params.nTargetSpacingNEW;
    int64_t nActualSpacing = pindexLast->GetBlockTime() - nFirstBlockTime;

    // Limit adjustment step
    if (nActualSpacing > nTargetSpacing*10)
        nActualSpacing = nTargetSpacing*10;

    // retarget with exponential moving toward target spacing
    const arith_uint256 bnTargetLimit = GetTargetLimit(pindexLast->GetBlockTime(), params, fProofOfStake);
    arith_uint256 bnNew;
    bnNew.SetCompact(pindexLast->nBits);
    int64_t nInterval = params.nTargetTimespan / nTargetSpacing;
    bnNew *= ((nInterval - 1) * nTargetSpacing + nActualSpacing + nActualSpacing);
    bnNew /= ((nInterval + 1) * nTargetSpacing);

    if (bnNew <= 0 || bnNew > bnTargetLimit)
        bnNew = bnTargetLimit;

    return bnNew.GetCompact();
}

unsigned int GetNextTargetRequired(const CBlockIndex* pindexLast, const CBlockHeader *pblock, const Consensus::Params& params, bool fProofOfStake)
{
    // Genesis block
    if (pindexLast == NULL)
        return UintToArith256(params.powLimit).GetCompact();

    /*
    unsigned int nTargetLimit = GetTargetLimit(pindexLast->GetBlockTime(), params, fProofOfStake).GetCompact();

    const CBlockIndex* pindexPrev = GetLastBlockIndex(pindexLast, fProofOfStake);

    if (pindexPrev->pprev == NULL)
        return nTargetLimit; // first block
    const CBlockIndex* pindexPrevPrev = GetLastBlockIndex(pindexPrev->pprev, fProofOfStake);
    if (pindexPrevPrev->pprev == NULL)
        return nTargetLimit; // second block
    */

    const CBlockIndex* pindexPrev = GetLastBlockIndex(pindexLast, fProofOfStake);
    const CBlockIndex* pindexPrevPrev = GetLastBlockIndex(pindexPrev->pprev, fProofOfStake);

    // No retargeting
    if (fProofOfStake) {
        if (params.fPoSNoRetargeting)
            return pindexLast->nBits;
    } else {
        if (params.fPowNoRetargeting)
            return pindexLast->nBits;
    }

    // Newest mode for PoSV3
    if (params.IsProtocolV3(pindexLast->GetBlockTime()))
        return CalculateNextTargetRequired(pindexPrev, pindexPrevPrev->GetBlockTime(), params, fProofOfStake);

    // New mode for PoS
    if (pindexLast->nHeight >= params.nLastPOWBlock)
        return GetPoSV1Difficulty(pindexLast, params, pblock); 

    int DiffMode = 1;
    if (pindexLast->nHeight + 1 >= params.nDifficultyKGW)
        DiffMode = 2;
    if (pindexLast->nHeight + 1 >= params.nDifficultyDigiShield)
        DiffMode = 3;

    if (DiffMode == 1) 
        return GetNextWorkRequired_V1(pindexLast, params, pblock);
    else if	(DiffMode == 2)
        return GetNextWorkRequired_V2(pindexLast, params, pblock);
    else if	(DiffMode == 3) 
        return GetNextWorkRequired_V3(pindexLast, params, pblock);
    return GetNextWorkRequired_V2(pindexLast, params, pblock);
}

bool CheckProofOfWork(uint256 hash, unsigned int nBits, const Consensus::Params& params)
{
    bool fNegative;
    bool fOverflow;
    arith_uint256 bnTarget;

    bnTarget.SetCompact(nBits, &fNegative, &fOverflow);

    // Check range
    if (fNegative || bnTarget == 0 || fOverflow || bnTarget > UintToArith256(params.powLimit))
        return false;

    // Check proof of work matches claimed amount
    if (UintToArith256(hash) > bnTarget)
        return false;

    return true;
}
