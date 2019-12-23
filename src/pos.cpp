// Copyright (c) 2014-2018 The BlackCoin Developers
// Copyright (c) 2011-2013 The PPCoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

// arith_uint512 patch by Navcoin
// Copyright (c) 2017-2019 The Navcoin developers

#include "pos.h"

#include "chain.h"
#include "chainparams.h"
#include "clientversion.h"
#include "coins.h"
#include "hash.h"
#include "main.h"
#include "uint256.h"
#include "primitives/transaction.h"
#include <stdio.h>
#include "util.h"

// Check whether the coinstake timestamp meets protocol
bool CheckCoinStakeTimestamp(int64_t nTimeBlock, int64_t nTimeTx)
{
    const Consensus::Params& params = Params().GetConsensus();
    if (params.IsProtocolV3(nTimeBlock))
        return (nTimeBlock == nTimeTx) && ((nTimeTx & params.nStakeTimestampMask) == 0);
    else
        return (nTimeBlock == nTimeTx);
}

// Simplified version of CheckCoinStakeTimestamp() to check header-only timestamp
bool CheckStakeBlockTimestamp(int64_t nTimeBlock)
{
   return CheckCoinStakeTimestamp(nTimeBlock, nTimeBlock);
}

/* PoSV: Coin-aging function
 * =================================================
 * WARNING
 * =================================================
 * The parameters used in this function are the
 * solutions to a set of intricate mathematical
 * equations chosen specifically to incentivise
 * owners of Potcoin to participate in minting.
 * These parameters are also affected by the values
 * assigned to other variables such as expected
 * block confirmation time.
 * If you are merely forking the source code of
 * Potcoin, it's highly UNLIKELY that this set of
 * parameters work for your purpose. In particular,
 * if you have tweaked the values of other variables,
 * this set of parameters are certainly no longer
 * valid. You should revert back to the linear
 * function above or the security of your network
 * will be significantly impaired.
 * In short, do not use or change this function
 * unless you have spoken to the author.
 * =================================================
 * DO NOT USE OR CHANGE UNLESS YOU ABSOLUTELY
 * KNOW WHAT YOU ARE DOING.
 * =================================================
 */
int64_t GetCoinAgeWeight(int64_t nIntervalBeginning, int64_t nIntervalEnd)
{
    if (nIntervalBeginning <= 0)
    {
        // printf("WARNING *** GetCoinAgeWeight: nIntervalBeginning (0x%016lx) <= 0\n", nIntervalBeginning);
        return 0;
    }

    int64_t nSeconds = max((int64_t)0, nIntervalEnd - nIntervalBeginning - (int64_t)Params().GetConsensus().nStakeMinAge);
    double days = double(nSeconds) / (24 * 60 * 60);
    double weight = 0;

    if (days <= 7)
    {
        weight = -0.00408163 * std::pow(days, 3) + 0.05714286 * std::pow(days, 2) + days;
    }
    else
    {
        weight = 8.4 * log(days) - 7.94564525;
    }

    return min((int64_t)(weight * 24 * 60 * 60), (int64_t)Params().GetConsensus().nStakeMaxAge);
} 

bool GetCoinAge(CTransaction& tx, uint64_t& nCoinAge)
{
    const Consensus::Params& params = Params().GetConsensus();
    arith_uint256 bnCentSecond = 0; // coin age in the unit of cent-seconds
    nCoinAge = 0;

    if (tx.IsCoinBase())
        return true;

    for (const auto& txin : tx.vin)
    {
        // First try finding the previous transaction in database
        CTransaction txPrev;
        uint256 hashBlock = uint256();

        if (!GetTransaction(txin.prevout.hash, txPrev, params, hashBlock, true))
            continue; // previous transaction not in main chain

        if (mapBlockIndex.count(hashBlock) == 0)
            return false; // Block not found

        CBlockIndex* pblockindex = mapBlockIndex[hashBlock];

        if (pblockindex->nTime + params.nStakeMinAge > tx.nTime)
            continue; // only count coins meeting min age requirement

        // deal with missing timestamps in PoW blocks
        if (txPrev.nTime == 0)
            txPrev.nTime = pblockindex->nTime;

        if (tx.nTime < txPrev.nTime)
            return false;  // Transaction timestamp violation

        int64_t nValueIn = txPrev.vout[txin.prevout.n].nValue;
        int64_t nTimeWeight = GetCoinAgeWeight(txPrev.nTime, tx.nTime);
        bnCentSecond += arith_uint256(nValueIn) * nTimeWeight / CENT;

        LogPrint("coinage", "coin age nValueIn=%d nTimeDiff=%d bnCentSecond=%s\n", nValueIn, tx.nTime - txPrev.nTime, bnCentSecond.ToString());
    }

    arith_uint256 bnCoinDay = bnCentSecond * CENT / COIN / (24 * 60 * 60);
    LogPrint("coinage", "coin age bnCoinDay=%s\n", bnCoinDay.ToString());
    nCoinAge = bnCoinDay.GetLow64();

    return true;
}

// Get the last stake modifier and its generation time from a given block
static bool GetLastStakeModifier(const CBlockIndex* pindex, uint64_t& nStakeModifier, int64_t& nModifierTime)
{
    if (!pindex)
        return error("GetLastStakeModifier: null pindex");
    while (pindex && pindex->pprev && !pindex->GeneratedStakeModifier())
        pindex = pindex->pprev;
    if (!pindex->GeneratedStakeModifier()){
        nStakeModifier = 0;
        nModifierTime = pindex->GetBlockTime();
        return true;
    }
    nStakeModifier = pindex->nStakeModifier;
    nModifierTime = pindex->GetBlockTime();
    return true;
}

// Get selection interval section (in seconds)
static int64_t GetStakeModifierSelectionIntervalSection(int nSection)
{
    assert (nSection >= 0 && nSection < 64);
    return (Params().GetConsensus().nModifierInterval * 63 / (63 + ((63 - nSection) * (MODIFIER_INTERVAL_RATIO - 1))));
}

// Get stake modifier selection interval (in seconds)
static int64_t GetStakeModifierSelectionInterval()
{
    int64_t nSelectionInterval = 0;
    for (int nSection=0; nSection<64; nSection++)
        nSelectionInterval += GetStakeModifierSelectionIntervalSection(nSection);
    return nSelectionInterval;
}

// select a block from the candidate blocks in vSortedByTimestamp, excluding
// already selected blocks in vSelectedBlocks, and with timestamp up to
// nSelectionIntervalStop.
static bool SelectBlockFromCandidates(vector<pair<int64_t, uint256> >& vSortedByTimestamp, map<uint256, const CBlockIndex*>& mapSelectedBlocks,
    int64_t nSelectionIntervalStop, uint64_t nStakeModifierPrev, const CBlockIndex** pindexSelected)
{
    bool fSelected = false;
    arith_uint256 hashBest = 0;
    *pindexSelected = (const CBlockIndex*) 0;
    for (const auto& item : vSortedByTimestamp)
    {
        if (!mapBlockIndex.count(item.second))
            return error("SelectBlockFromCandidates: failed to find block index for candidate block %s", item.second.ToString());
        const CBlockIndex* pindex = mapBlockIndex[item.second];
        if (fSelected && pindex->GetBlockTime() > nSelectionIntervalStop) {
            break;
        }

        if (mapSelectedBlocks.count(pindex->GetBlockHash()) > 0)
            continue;
        // compute the selection hash by hashing its proof-hash and the
        // previous proof-of-stake modifier
        CDataStream ss(SER_GETHASH, 0);
        ss << pindex->hashProofOfStake << nStakeModifierPrev;
        arith_uint256 hashSelection = UintToArith256(Hash(ss.begin(), ss.end()));

        // the selection hash is divided by 2**32 so that proof-of-stake block
        // is always favored over proof-of-work block. this is to preserve
        // the energy efficiency property
        if (pindex->IsProofOfStake())
            hashSelection >>= 32;

        if (fSelected && hashSelection < hashBest)
        {
            hashBest = hashSelection;
            *pindexSelected = (const CBlockIndex*) pindex;
        }
        else if (!fSelected)
        {
            fSelected = true;
            hashBest = hashSelection;
            *pindexSelected = (const CBlockIndex*) pindex;
        }
    }
    if (fDebug)
        LogPrintf("SelectBlockFromCandidates: selection hash=%s\n", hashBest.ToString());
    return fSelected;
}

// Stake Modifier (hash modifier of proof-of-stake):
// The purpose of stake modifier is to prevent a txout (coin) owner from
// computing future proof-of-stake generated by this txout at the time
// of transaction confirmation. To meet kernel protocol, the txout
// must hash with a future stake modifier to generate the proof.
// Stake modifier consists of bits each of which is contributed from a
// selected block of a given block group in the past.
// The selection of a block is based on a hash of the block's proof-hash and
// the previous stake modifier.
// Stake modifier is recomputed at a fixed time interval instead of every 
// block. This is to make it difficult for an attacker to gain control of
// additional bits in the stake modifier, even after generating a chain of
// blocks.
bool ComputeNextStakeModifier(const CBlockIndex* pindexPrev, uint64_t& nStakeModifier, bool& fGeneratedStakeModifier)
{
    // Peercoin 0.8
    const Consensus::Params& params = Params().GetConsensus();
    nStakeModifier = 0;
    fGeneratedStakeModifier = false;
    if (!pindexPrev)
    {
        fGeneratedStakeModifier = true;
        return true; // genesis block's modifier is 0
    }
    // First find current stake modifier and its generation block time
    // if it's not old enough, return the same stake modifier
    int64_t nModifierTime = 0;
    if (!GetLastStakeModifier(pindexPrev, nStakeModifier, nModifierTime))
        return error("ComputeNextStakeModifier: unable to get last modifier");
    if (fDebug)
        LogPrint("stakemodifier", "ComputeNextStakeModifier: prev modifier=0x%016x time=%s epoch=%u\n", nStakeModifier, DateTimeStrFormat(nModifierTime), (unsigned int)nModifierTime);
    if (nModifierTime / params.nModifierInterval >= pindexPrev->GetBlockTime() / params.nModifierInterval) {
        if (fDebug)
            LogPrint("stakemodifier", "ComputeNextStakeModifier: no new interval keep current modifier: pindexPrev nHeight=%d nTime=%u\n", pindexPrev->nHeight, (unsigned int)pindexPrev->GetBlockTime());
        return true;
    }

    // Sort candidate blocks by timestamp
    vector<pair<int64_t, uint256> > vSortedByTimestamp;
    vSortedByTimestamp.reserve(64 * params.nModifierInterval / params.nTargetSpacing);
    int64_t nSelectionInterval = GetStakeModifierSelectionInterval();
    int64_t nSelectionIntervalStart = (pindexPrev->GetBlockTime() / params.nModifierInterval) * params.nModifierInterval - nSelectionInterval;
    const CBlockIndex* pindex = pindexPrev;
    while (pindex && pindex->GetBlockTime() >= nSelectionIntervalStart)
    {
        vSortedByTimestamp.push_back(make_pair(pindex->GetBlockTime(), pindex->GetBlockHash()));
        pindex = pindex->pprev;
    }
    int nHeightFirstCandidate = pindex ? (pindex->nHeight + 1) : 0;

    // Shuffle before sort
    for(int i = vSortedByTimestamp.size() - 1; i > 1; --i)
    std::swap(vSortedByTimestamp[i], vSortedByTimestamp[GetRand(i)]);

    sort(vSortedByTimestamp.begin(), vSortedByTimestamp.end(), [] (const pair<int64_t, uint256> &a, const pair<int64_t, uint256> &b)
    {
        if (a.first != b.first)
            return a.first < b.first;
        // Timestamp equals - compare block hashes
        const uint32_t *pa = a.second.GetDataPtr();
        const uint32_t *pb = b.second.GetDataPtr();
        int cnt = 256 / 32;
        do {
            --cnt;
            if (pa[cnt] != pb[cnt])
                return pa[cnt] < pb[cnt];
        } while(cnt);
            return false; // Elements are equal
    });

    // Select 64 blocks from candidate blocks to generate stake modifier
    uint64_t nStakeModifierNew = 0;
    int64_t nSelectionIntervalStop = nSelectionIntervalStart;
    map<uint256, const CBlockIndex*> mapSelectedBlocks;
    for (int nRound=0; nRound<min(64, (int)vSortedByTimestamp.size()); nRound++)
    {
        // add an interval section to the current selection round
        nSelectionIntervalStop += GetStakeModifierSelectionIntervalSection(nRound);
        // select a block from the candidates of current round
        if (!SelectBlockFromCandidates(vSortedByTimestamp, mapSelectedBlocks, nSelectionIntervalStop, nStakeModifier, &pindex))
            return error("ComputeNextStakeModifier: unable to select block at round %d", nRound);
        // write the entropy bit of the selected block
        nStakeModifierNew |= (((uint64_t)pindex->GetStakeEntropyBit()) << nRound);
        // add the selected block from candidates to selected list
        mapSelectedBlocks.insert(make_pair(pindex->GetBlockHash(), pindex));
        if (fDebug) {
            LogPrint("stakemodifier", "ComputeNextStakeModifier: selected round %d stop=%s height=%d bit=%d\n", nRound, DateTimeStrFormat("%Y-%m-%d %H:%M:%S", nSelectionIntervalStop), pindex->nHeight, pindex->GetStakeEntropyBit());
        }
    }

    // Print selection map for visualization of the selected blocks
    if (fDebug)
    {
        string strSelectionMap = "";
        // '-' indicates proof-of-work blocks not selected
        strSelectionMap.insert(0, pindexPrev->nHeight - nHeightFirstCandidate + 1, '-');
        pindex = pindexPrev;
        while (pindex && pindex->nHeight >= nHeightFirstCandidate)
        {
            // '=' indicates proof-of-stake blocks not selected
            if (pindex->IsProofOfStake())
                strSelectionMap.replace(pindex->nHeight - nHeightFirstCandidate, 1, "=");
            pindex = pindex->pprev;
        }
        for (const auto& item : mapSelectedBlocks)
        {
            // 'S' indicates selected proof-of-stake blocks
            // 'W' indicates selected proof-of-work blocks
            strSelectionMap.replace(item.second->nHeight - nHeightFirstCandidate, 1, item.second->IsProofOfStake()? "S" : "W");
        }
        LogPrint("stakemodifier", "ComputeNextStakeModifier: selection height [%d, %d] map %s\n", nHeightFirstCandidate, pindexPrev->nHeight, strSelectionMap.c_str());
    }
    if (fDebug)
        LogPrint("stakemodifier", "ComputeNextStakeModifier: new modifier=0x%016x time=%s\n", nStakeModifierNew, DateTimeStrFormat("%Y-%m-%d %H:%M:%S", pindexPrev->GetBlockTime()));

    nStakeModifier = nStakeModifierNew;
    fGeneratedStakeModifier = true;
    return true;
}

// The stake modifier used to hash for a stake kernel is chosen as the stake
// modifier about a selection interval later than the coin generating the kernel
static bool GetKernelStakeModifier(CBlockIndex* pindexPrev, uint256 hashBlockFrom, uint64_t& nStakeModifier, int& nStakeModifierHeight, int64_t& nStakeModifierTime, bool fPrintProofOfStake)
{
    // Peercoin 0.8
    const Consensus::Params& params = Params().GetConsensus();
    nStakeModifier = 0;
    if (!mapBlockIndex.count(hashBlockFrom))
        return error("GetKernelStakeModifier() : block not indexed");
    const CBlockIndex* pindexFrom = mapBlockIndex[hashBlockFrom];
    nStakeModifierHeight = pindexFrom->nHeight;
    nStakeModifierTime = pindexFrom->GetBlockTime();
    int64_t nStakeModifierSelectionInterval = GetStakeModifierSelectionInterval();

    // we need to iterate index forward but we cannot depend on chainActive.Next()
    // because there is no guarantee that we are checking blocks in active chain.
    // So, we construct a temporary chain that we will iterate over.
    // pindexFrom - this block contains coins that are used to generate PoS
    // pindexPrev - this is a block that is previous to PoS block that we are checking, you can think of it as tip of our chain
    std::vector<CBlockIndex*> tmpChain;
    int32_t nDepth = pindexPrev->nHeight - (pindexFrom->nHeight-1); // -1 is used to also include pindexFrom
    tmpChain.reserve(nDepth);
    CBlockIndex* it = pindexPrev;
    for (int i=1; i<=nDepth && !chainActive.Contains(it); i++) {
        tmpChain.push_back(it);
        it = it->pprev;
    }
    std::reverse(tmpChain.begin(), tmpChain.end());
    size_t n = 0;

    const CBlockIndex* pindex = pindexFrom;
    // loop to find the stake modifier later by a selection interval
    while (nStakeModifierTime < pindexFrom->GetBlockTime() + nStakeModifierSelectionInterval)
    {
        const CBlockIndex* old_pindex = pindex;
        pindex = (!tmpChain.empty() && pindex->nHeight >= tmpChain[0]->nHeight - 1)? tmpChain[n++] : chainActive.Next(pindex);
        if (n > tmpChain.size() || pindex == NULL) // check if tmpChain[n+1] exists
        {   // reached best block; may happen if node is behind on block chain
            if (fPrintProofOfStake || (old_pindex->GetBlockTime() + params.nStakeMinAge - nStakeModifierSelectionInterval > GetAdjustedTime()))
                return error("GetKernelStakeModifier() : reached best block %s at height %d from block %s",
                    old_pindex->GetBlockHash().ToString(), old_pindex->nHeight, hashBlockFrom.ToString());
            else
                return false;
        }
        if (pindex->GeneratedStakeModifier())
        {
            nStakeModifierHeight = pindex->nHeight;
            nStakeModifierTime = pindex->GetBlockTime();
        }
    }
    nStakeModifier = pindex->nStakeModifier;
    return true;
}

// Stake Modifier (hash modifier of proof-of-stake):
// The purpose of stake modifier is to prevent a txout (coin) owner from
// computing future proof-of-stake generated by this txout at the time
// of transaction confirmation. To meet kernel protocol, the txout
// must hash with a future stake modifier to generate the proof.
uint256 ComputeStakeModifierV2(const CBlockIndex* pindexPrev, const uint256& kernel)
{
    if (!pindexPrev)
        return uint256(); // genesis block's modifier is 0

    CHashWriter ss(SER_GETHASH, 0);
    ss << kernel << pindexPrev->nStakeModifierV2;
    return ss.GetHash();
}

// peercoin kernel protocol (v0.3)
// coinstake must meet hash target according to the protocol:
// kernel (input 0) must meet the formula
//     hash(nStakeModifier + txPrev.block.nTime + txPrev.offset + txPrev.nTime + txPrev.vout.n + nTime) < bnTarget * nCoinDayWeight
// this ensures that the chance of getting a coinstake is proportional to the
// amount of coin age one owns.
// The reason this hash is chosen is the following:
//   nStakeModifier: scrambles computation to make it very difficult to precompute
//                  future proof-of-stake at the time of the coin's confirmation
//   txPrev.block.nTime: prevent nodes from guessing a good timestamp to
//                       generate transaction for future advantage
//   txPrev.offset: offset of txPrev inside block, to reduce the chance of 
//                  nodes generating coinstake at the same time
//   txPrev.nTime: reduce the chance of nodes generating coinstake at the same
//                 time
//   txPrev.vout.n: output number of txPrev, to reduce the chance of nodes
//                  generating coinstake at the same time
//   block/tx hash should not be used here as they can be generated in vast
//   quantities so as to generate blocks faster, degrading the system back into
//   a proof-of-work situation.
//
bool CheckStakeKernelHashV1(unsigned int nBits, CBlockIndex* pindexPrev, const CBlockHeader& blockFrom, unsigned int nTxPrevOffset, const CTransaction& txPrev, const COutPoint& prevout, unsigned int nTimeTx, uint256& hashProofOfStake, bool fPrintProofOfStake)
{
    const Consensus::Params& params = Params().GetConsensus();

    // Deal with missing timestamps in PoW blocks
    unsigned int nTimeTxPrev = txPrev.nTime;
    unsigned int nTimeBlockFrom = blockFrom.GetBlockTime();
    if (nTimeTxPrev == 0)
        nTimeTxPrev = nTimeBlockFrom;
    if (nTimeTx < nTimeTxPrev)  // Transaction timestamp violation
        return error("CheckStakeKernelHash() : nTime violation");

    if (nTimeBlockFrom + params.nStakeMinAge > nTimeTx) // Min age requirement
        return error("CheckStakeKernelHash() : min age violation");

    // Base target
    arith_uint256 targetProofOfStake;
    targetProofOfStake.SetCompact(nBits);

    // Weighted target
    int64_t nValueIn = txPrev.vout[prevout.n].nValue;
    arith_uint512 bnWeight = arith_uint512(nValueIn) * GetCoinAgeWeight((int64_t)nTimeTxPrev, (int64_t)nTimeTx) / COIN / (24 * 60 * 60);

    // We need to convert to uint512 to prevent overflow
    base_uint<512> targetProofOfStake512(targetProofOfStake.GetHex());
    targetProofOfStake512 *= bnWeight;

    uint256 hashBlockFrom = blockFrom.GetHash();

    // Calculate hash
    CDataStream ss(SER_GETHASH, 0);
    uint64_t nStakeModifier = 0;
    int nStakeModifierHeight = 0;
    int64_t nStakeModifierTime = 0;

    if (!GetKernelStakeModifier(pindexPrev, hashBlockFrom, nStakeModifier, nStakeModifierHeight, nStakeModifierTime, fPrintProofOfStake)){
        LogPrintf("CheckStakeKernelHash() : GetKernelStakeModifier failed\n");
        return false;
    }

    ss << nStakeModifier;
    ss << nTimeBlockFrom << nTxPrevOffset << nTimeTxPrev << prevout.n << nTimeTx;
    hashProofOfStake = Hash(ss.begin(), ss.end());
    if (fPrintProofOfStake)
    {
        LogPrintf("CheckStakeKernelHash() : using modifier 0x%016x at height=%d timestamp=%s for block from height=%d timestamp=%s\n",
            nStakeModifier, nStakeModifierHeight,
            DateTimeStrFormat(nStakeModifierTime),
            mapBlockIndex[hashBlockFrom]->nHeight,
            DateTimeStrFormat(blockFrom.GetBlockTime()));
        LogPrintf("CheckStakeKernelHash() : check modifier=0x%016x nTimeBlockFrom=%u nTxPrevOffset=%u nTimeTxPrev=%u nPrevout=%u nTimeTx=%u hashProofOfStake=%s targetProofOfStake=%s\n",
            nStakeModifier,
            nTimeBlockFrom, nTxPrevOffset, nTimeTxPrev, prevout.n, nTimeTx,
            hashProofOfStake.ToString(), targetProofOfStake512.ToString());
    }

    // We need to convert type so it can be compared to target
    base_uint<512> hashProofOfStake512(hashProofOfStake.GetHex());

    // Now check if proof-of-stake hash meets target protocol
    if (hashProofOfStake512 > targetProofOfStake512)
        return false;

    if (fDebug && !fPrintProofOfStake)
    {
        LogPrintf("CheckStakeKernelHash() : using modifier=0x%016x at height=%d timestamp=%s for block from height=%d timestamp=%s\n",
            nStakeModifier, nStakeModifierHeight, 
            DateTimeStrFormat(nStakeModifierTime),
            mapBlockIndex[hashBlockFrom]->nHeight,
            DateTimeStrFormat(blockFrom.GetBlockTime()));
        LogPrintf("CheckStakeKernelHash() : pass modifier=0x%016x nTimeBlockFrom=%u nTxPrevOffset=%u nTimeTxPrev=%u nPrevout=%u nTimeTx=%u hashProofOfStake=%s targetProofOfStake=%s\n",
            nStakeModifier,
            nTimeBlockFrom, nTxPrevOffset, nTimeTxPrev, prevout.n, nTimeTx,
            hashProofOfStake.ToString(), targetProofOfStake512.ToString());
    }
    return true;
}

// BlackCoin kernel protocol v3
// coinstake must meet hash target according to the protocol:
// kernel (input 0) must meet the formula
//     hash(nStakeModifier + txPrev.nTime + txPrev.vout.hash + txPrev.vout.n + nTime) < bnTarget * nWeight
// this ensures that the chance of getting a coinstake is proportional to the
// amount of coins one owns.
// The reason this hash is chosen is the following:
//   nStakeModifier: scrambles computation to make it very difficult to precompute
//                   future proof-of-stake
//   txPrev.nTime: slightly scrambles computation
//   txPrev.vout.hash: hash of txPrev, to reduce the chance of nodes
//                     generating coinstake at the same time
//   txPrev.vout.n: output number of txPrev, to reduce the chance of nodes
//                  generating coinstake at the same time
//   nTime: current timestamp
//   block/tx hash should not be used here as they can be generated in vast
//   quantities so as to generate blocks faster, degrading the system back into
//   a proof-of-work situation.
//
bool CheckStakeKernelHashV2(const CBlockIndex* pindexPrev, unsigned int nBits, const CTransaction& txPrev, const COutPoint& prevout, unsigned int nTimeTx, uint256& hashProofOfStake, bool fPrintProofOfStake)
{
    if (nTimeTx < txPrev.nTime)  // Transaction timestamp violation
        return error("CheckStakeKernelHash() : nTime violation");

    // Base target
    arith_uint256 targetProofOfStake;
    targetProofOfStake.SetCompact(nBits);

    // Weighted target
    int64_t nValueIn = txPrev.vout[prevout.n].nValue;
    if (nValueIn == 0)
        return error("CheckStakeKernelHash() : nValueIn = 0");
    arith_uint512 bnWeight = arith_uint512(nValueIn);

    // We need to convert to uint512 to prevent overflow
    base_uint<512> targetProofOfStake512(targetProofOfStake.GetHex());
    targetProofOfStake512 *= bnWeight;

    uint256 nStakeModifierV2 = pindexPrev->nStakeModifierV2;

    // Calculate hash
    CDataStream ss(SER_GETHASH, 0);
    ss << nStakeModifierV2;
    ss << txPrev.nTime << prevout.hash << prevout.n << nTimeTx;

    hashProofOfStake = Hash(ss.begin(), ss.end());

    if (fPrintProofOfStake)
    {
        LogPrintf("CheckStakeKernelHash() : nStakeModifierV2=%s, txPrev.nTime=%u, txPrev.vout.hash=%s, txPrev.vout.n=%u, nTime=%u, hashProofOfStake=%s, targetProofOfStake=%s\n",
            nStakeModifierV2.GetHex().c_str(),
            txPrev.nTime, prevout.hash.ToString(), prevout.n, nTimeTx,
            hashProofOfStake.ToString(), targetProofOfStake512.ToString());
    }

    // We need to convert type so it can be compared to target
    base_uint<512> hashProofOfStake512(hashProofOfStake.GetHex());

    // Now check if proof-of-stake hash meets target protocol
    if (hashProofOfStake512 > targetProofOfStake512)
        return false;

    if (fDebug && !fPrintProofOfStake)
    {
        LogPrintf("CheckStakeKernelHash() : nStakeModifierV2=%s, txPrev.nTime=%u, txPrev.vout.hash=%s, txPrev.vout.n=%u, nTime=%u, hashProofOfStake=%s, targetProofOfStake=%s\n",
            nStakeModifierV2.GetHex().c_str(),
            txPrev.nTime, prevout.hash.ToString(), prevout.n, nTimeTx,
            hashProofOfStake.ToString(), targetProofOfStake512.ToString());
    }

    return true;
}

bool CheckStakeKernelHash(CBlockIndex* pindexPrev, unsigned int nBits, CBlockHeader& blockFrom, unsigned int nTxPrevOffset, const CTransaction& txPrev, const COutPoint& prevout, unsigned int nTimeTx, uint256& hashProofOfStake, bool fPrintProofOfStake)
{
    if (Params().GetConsensus().IsProtocolV3(pindexPrev->GetBlockTime()))
        return CheckStakeKernelHashV2(pindexPrev, nBits, txPrev, prevout, nTimeTx, hashProofOfStake, fPrintProofOfStake);
    else
        return CheckStakeKernelHashV1(nBits, pindexPrev, blockFrom, nTxPrevOffset, txPrev, prevout, nTimeTx, hashProofOfStake, fPrintProofOfStake);
}

// Check kernel hash target and coinstake signature
bool CheckProofOfStake(CBlockIndex* pindexPrev, const CTransaction& tx, unsigned int nBits, uint256& hashProofOfStake, CValidationState &state)
{
    const Consensus::Params& params = Params().GetConsensus();

    if (!tx.IsCoinStake())
        return error("CheckProofOfStake() : called on non-coinstake %s", tx.GetHash().ToString());

    // Kernel (input 0) must match the stake hash target per coin age (nBits)
    const CTxIn& txin = tx.vin[0];

    // Get transaction index for the previous transaction
    CDiskTxPos postx;
    if (!pblocktree->ReadTxIndex(txin.prevout.hash, postx))
        return error("CheckProofOfStake() : tx index not found");  // tx index not found

    // Read txPrev and header of its block
    CBlockHeader header;
    CTransaction txPrev;
    {
        CAutoFile file(OpenBlockFile(postx, true), SER_DISK, CLIENT_VERSION);
        try {
            file >> header;
            fseek(file.Get(), postx.nTxOffset, SEEK_CUR);
            file >> txPrev;
        } catch (std::exception &e) {
            return error("%s() : deserialize or I/O error in CheckProofOfStake()", __PRETTY_FUNCTION__);
        }
        if (txPrev.GetHash() != txin.prevout.hash)
            return error("%s() : txid mismatch in CheckProofOfStake()", __PRETTY_FUNCTION__);
    }

    if (params.IsProtocolV3(tx.nTime)) {
        // Try finding the previous transaction in database
        uint256 hashBlock = uint256();
        if (!GetTransaction(txin.prevout.hash, txPrev, params, hashBlock, true))
            return state.DoS(100, error("CheckProofOfStake() : read txPrev failed")); // previous transaction not in main chain, may occur during initial download
        if (mapBlockIndex.count(hashBlock) == 0)
            return state.DoS(100, error("CheckProofOfStake() : read block failed")); // unable to read block of previous transaction

        CBlockIndex* pblockindex = mapBlockIndex[hashBlock];

        // Check minimum age requirement
        if (pindexPrev->nHeight + 1 - pblockindex->nHeight < params.nCoinbaseMaturity)
            return state.DoS(100, error("CheckProofOfStake() : stake prevout is not mature, expecting %i and only matured to %i", params.nCoinbaseMaturity, pindexPrev->nHeight + 1 - pblockindex->nHeight));
    }

    // Verify signature
    if (!VerifySignature(txPrev, tx, 0, SCRIPT_VERIFY_P2SH, 0))
       return state.DoS(100, error("CheckProofOfStake() : VerifySignature failed on coinstake %s", tx.GetHash().ToString()));

    // Verify kernel hash
    if (!CheckStakeKernelHash(pindexPrev, nBits, header, txin.prevout.n, txPrev, txin.prevout, tx.nTime, hashProofOfStake, fDebug))
       return state.DoS(1, error("CheckProofOfStake() : INFO: check kernel failed on coinstake %s", tx.GetHash().ToString())); // may occur during initial download or if behind on block chain sync

    return true;
}

bool VerifySignature(const CTransaction& txFrom, const CTransaction& txTo, unsigned int nIn, unsigned int flags, int nHashType)
{
    assert(nIn < txTo.vin.size());
    const CTxIn& txin = txTo.vin[nIn];
    if (txin.prevout.n >= txFrom.vout.size())
        return false;
    const CTxOut& txout = txFrom.vout[txin.prevout.n];

    if (txin.prevout.hash != txFrom.GetHash())
        return false;

    return VerifyScript(txin.scriptSig, txout.scriptPubKey, flags, TransactionSignatureChecker(&txTo, nIn, 0),  NULL);
}

bool CheckKernel(CBlockIndex* pindexPrev, unsigned int nBits, uint32_t nTime, const COutPoint& prevout, int64_t* pBlockTime)
{
    const Consensus::Params& params = Params().GetConsensus();
    uint256 hashProofOfStake = uint256();

    // Get transaction index for the previous transaction
    CDiskTxPos postx;
    if (!pblocktree->ReadTxIndex(prevout.hash, postx))
        return error("CheckKernel() : tx index not found");  // tx index not found

    // Read txPrev and header of its block
    CBlockHeader header;
    CTransaction txPrev;
    {
        CAutoFile file(OpenBlockFile(postx, true), SER_DISK, CLIENT_VERSION);
        try {
            file >> header;
            fseek(file.Get(), postx.nTxOffset, SEEK_CUR);
            file >> txPrev;
        } catch (std::exception &e) {
            return error("%s() : deserialize or I/O error in CheckKernel()", __PRETTY_FUNCTION__);
        }
        if (txPrev.GetHash() != prevout.hash)
            return error("%s() : txid mismatch in CheckKernel()", __PRETTY_FUNCTION__);
    }

    // Try finding the previous transaction in database
    uint256 hashBlock = uint256();
    CTransaction txPrevDummy;
    if (!GetTransaction(prevout.hash, txPrevDummy, params, hashBlock, true)) {
        LogPrintf("CheckKernel() : could not find previous transaction %s\n", prevout.hash.ToString());
        return false;
    }
    if (mapBlockIndex.count(hashBlock) == 0) {
        LogPrintf("CheckKernel() : could not find block of previous transaction %s\n", hashBlock.ToString());
        return false;
    }

    CBlockIndex* pblockindex = mapBlockIndex[hashBlock];

    if (params.IsProtocolV3(nTime)) {
        // Check minimum age requirement
        if (pindexPrev->nHeight + 1 - pblockindex->nHeight < params.nCoinbaseMaturity) {
            LogPrintf("CheckKernel() : stake prevout is not mature in block %s\n", hashBlock.ToString());
            return false;
        }
    }

    if (pBlockTime)
        *pBlockTime = pblockindex->GetBlockTime();

    return CheckStakeKernelHash(pindexPrev, nBits, header, prevout.n, txPrev, prevout, nTime, hashProofOfStake);
}
