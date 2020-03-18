// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2015 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "chainparams.h"
#include "consensus/merkle.h"

#include "tinyformat.h"
#include "util.h"
#include "utilstrencodings.h"
#include "streams.h"

#include <assert.h>

#include <boost/assign/list_of.hpp>

#include "chainparamsseeds.h"

using namespace std;

static CBlock CreateGenesisBlock(const char* pszTimestamp, const CScript& genesisOutputScript, uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    CMutableTransaction txNew;
    txNew.nVersion = 1;
    txNew.nTime = nTime;
    txNew.vin.resize(1);
    txNew.vout.resize(1);
    txNew.vin[0].scriptSig = CScript() << 486604799 << CScriptNum(4) << vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
    txNew.vout[0].nValue = genesisReward;
    txNew.vout[0].scriptPubKey = genesisOutputScript;

    CBlock genesis;
    genesis.nTime    = nTime;
    genesis.nBits    = nBits;
    genesis.nNonce   = nNonce;
    genesis.nVersion = nVersion;
    genesis.vtx.push_back(txNew);
    genesis.hashPrevBlock.SetNull();
    genesis.hashMerkleRoot = BlockMerkleRoot(genesis);
    return genesis;
}

/**
 * Build the genesis block. Note that the output of its generation
 * transaction cannot be spent since it did not originally exist in the
 * database.
 */
static CBlock CreateGenesisBlock(uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    const char* pszTimestamp = "Banks Aren't Accepting Legal Marijuana Money. Here's Why";
    const CScript genesisOutputScript = CScript() << ParseHex("040184710fa689ad5023690c80f3a49c8f13f8d45b8c857fbcbc8bc4a8e4d3eb4b10f4d4604fa08dce601aaf0f470216fe1b51850b4acf21b179c45070ac7b03a9") << OP_CHECKSIG;
    return CreateGenesisBlock(pszTimestamp, genesisOutputScript, nTime, nNonce, nBits, nVersion, genesisReward);
}

static CBlock CreateGenesisBlockTestnet(uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    const char* pszTimestamp = "Banks Aren't Accepting Legal Marijuana Money. Here's Why";
    const CScript genesisOutputScript = CScript() << ParseHex("040184710fa689ad5023690c80f3a49c8f13f8d45b8c857fbcbc8bc4a8e4d3eb4b10f4d4604fa08dce601aaf0f470216fe1b51850b4acf21b179c45070ac7b03a9") << OP_CHECKSIG;
    return CreateGenesisBlock(pszTimestamp, genesisOutputScript, nTime, nNonce, nBits, nVersion, genesisReward);
}

static CBlock CreateGenesisBlockRegtest(uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    const char* pszTimestamp = "New testnet for Potcoin!";
    const CScript genesisOutputScript = CScript() << ParseHex("040184710fa689ad5023690c80f3a49c8f13f8d45b8c857fbcbc8bc4a8e4d3eb4b10f4d4604fa08dce601aaf0f470216fe1b51850b4acf21b179c45070ac7b03a9") << OP_CHECKSIG;
    return CreateGenesisBlock(pszTimestamp, genesisOutputScript, nTime, nNonce, nBits, nVersion, genesisReward);
}

/*
void static MineNewGenesisBlock(const Consensus::Params& consensus, CBlock &genesis)
{
    LogPrintf("Searching for genesis block...\n");
    std::cout << "Searching for genesis block...\n";
    arith_uint256 hashTarget = UintToArith256(consensus.powLimit);

    while (true) {
        arith_uint256 thash = UintToArith256(genesis.GetHash());

		LogPrintf("Test Hash %s\n", thash.ToString().c_str());
        std::cout << "Test Hash = " << thash.ToString().c_str() << "\n";
		LogPrintf("Hash Target %s\n", hashTarget.ToString().c_str());
        std::cout << "Hash Target = " << hashTarget.ToString().c_str() << "\n";

      if (thash <= hashTarget)
            break;
        if ((genesis.nNonce & 0xFFF) == 0)
            LogPrintf("nonce %08X: hash = %s (target = %s)\n", genesis.nNonce, thash.ToString().c_str(), hashTarget.ToString().c_str());

        ++genesis.nNonce;
        if (genesis.nNonce == 0) {
            LogPrintf("NONCE WRAPPED, incrementing time\n");
            ++genesis.nTime;
        }
    }
    std::cout << "genesis.nTime = " << genesis.nTime << "\n";
    std::cout << "genesis.nNonce = " << genesis.nNonce<< "\n";
    std::cout << "genesis.nBits = " << genesis.nBits<< "\n";
    std::cout << "genesis.GetHash = " << genesis.GetHash().ToString().c_str()<< "\n";
    std::cout << "genesis.hashMerkleRoot = " << genesis.hashMerkleRoot.ToString().c_str()<< "\n";

    LogPrintf("genesis.nTime = %u \n",  genesis.nTime);
    LogPrintf("genesis.nNonce = %u \n",  genesis.nNonce);
    LogPrintf("genesis.nBits = %u \n",  genesis.nBits);
    LogPrintf("genesis.GetHash = %s\n",  genesis.GetHash().ToString().c_str());
    LogPrintf("genesis.hashMerkleRoot = %s\n",  genesis.hashMerkleRoot.ToString().c_str());

    exit(1);
}
*/

/**
 * Main network
 */
/**
 * What makes a good checkpoint block?
 * + Is surrounded by blocks with reasonable timestamps
 *   (no blocks before with a timestamp after, none after with
 *    timestamp before)
 * + Contains no strange transactions
 */

class CMainParams : public CChainParams {
public:
    CMainParams() {
        strNetworkID = "main";
        consensus.nMaxReorganizationDepth = 60 * 4; // Potcoin, equals to nCoinbaseMaturityNEW
        consensus.nMajorityEnforceBlockUpgrade = 750;
        consensus.nMajorityRejectBlockOutdated = 950;
        consensus.nMajorityWindow = 1000;
        consensus.powLimit = uint256S("00000fffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.posLimit = uint256S("00000fffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.posLimitV2 = uint256S("000000000000ffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nTargetTimespan = 108 * 40; // Potcoin: 3.5 days
        consensus.nTargetTimespanNEW = 40; // Potcoin: 40 seconds
        consensus.nTargetSpacing = 40; // Potcoin: 40 seconds
        consensus.nTargetSpacingNEW = 48; // Potcoin: 48 seconds
        consensus.nModifierInterval = 13 * 60; // Potcoin: nModifierInterval
        consensus.BIP34Height = -1;
        consensus.BIP34Hash = uint256();
        consensus.fPowAllowMinDifficultyBlocks = false;
        consensus.fPowNoRetargeting = false;
        consensus.fPoSNoRetargeting = false;
        consensus.nRuleChangeActivationThreshold = 1916; // 95% of 2016
        consensus.nMinerConfirmationWindow = 2016; // nTargetTimespan / nTargetSpacing
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601; // January 1, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999; // December 31, 2008

        consensus.nDifficultyKGW = 61798;
        consensus.nDifficultyDigiShield = 280000; 
        consensus.nCoinbaseMaturitySwitch = 280000;
        consensus.nCheckPOWFromnTime = 1414964233;
        consensus.nProtocolV3Time = std::numeric_limits<int64_t>::max();

        consensus.nLastPOWBlock = 974999;
        consensus.nStakeTimestampMask = 0xf; // 15
        consensus.nCoinbaseMaturity = 5; // Potcoin: 5 blocks
        consensus.nCoinbaseMaturityNEW = 60 * 4; // Potcoin: coinbase maturity after block 145000 - 240 blocks
        consensus.nStakeMinAge = 8 * 60 * 60; // Potcoin: 8 hours
        consensus.nStakeMaxAge = 365 * 24 * 60 * 60; // Potcoin: 365 days

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x0000000000000000000000000000000000000000000000000300c32884233faf");

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0x475e42eace5706469310b955d58d0f18376f65e32f5a6aec7348a8cd034df54e");

        /**
         * The message start string is designed to be unlikely to occur in normal data.
         * The characters are rarely used upper ASCII, not valid as UTF-8, and produce
         * a large 32-bit integer with any alignment.
         */
        pchMessageStart[0] = 0xfb;
        pchMessageStart[1] = 0xc0;
        pchMessageStart[2] = 0xb6;
        pchMessageStart[3] = 0xdb;
        nDefaultPort = 4200;
        nPruneAfterHeight = 100000;

        genesis = CreateGenesisBlock(1389688315, 471993, 0x1e0ffff0, 1, 420 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0xde36b0cb2a9c7d1d7ac0174d0a89918f874fabcf5f9741dd52cd6d04ee1335ec"));
        assert(genesis.hashMerkleRoot == uint256S("0xd5a08606e06eea7eae8a889dbcdcdd84917c10fc8e177ec013a9005305afe53d"));

        vSeeds.push_back(CDNSSeedData("potcoinseed.com", "dnsseed.potcoinseed.com"));

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,55);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,117);
        base58Prefixes[SECRET_KEY] = std::vector<unsigned char>(1,132);
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x04)(0x88)(0xB2)(0x1E).convert_to_container<std::vector<unsigned char> >();
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x04)(0x88)(0xAD)(0xE4).convert_to_container<std::vector<unsigned char> >();
        cashaddrPrefix = "potcoin";

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_main, pnSeed6_main + ARRAYLEN(pnSeed6_main));

        fMiningRequiresPeers = true;
        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        fMineBlocksOnDemand = false;
        fTestnetToBeDeprecatedFieldRPC = false;

        checkpointData = (CCheckpointData) {
                    boost::assign::map_list_of
                    ( 10, uint256S("0xc2818dc9bf6fd7fb692cc0886d39a724bb4e86fad095a62266bd015b9fbae04f"))
                    ( 17, uint256S("0x6c8818dd77bcaee6c3c775a34c0a84f349cc4db99e2c8b40ed7adb83b0184606"))
                    ( 22, uint256S("0x592cc1502043365de34d8c806fa2355e8f2ca47bfd568812c77547b4b72df744"))
                    ( 27, uint256S("0x49fc54fa7fe3939e57b83a468cb40333177b8e1ae1648a641ccc79d47ca68834"))
                    ( 35, uint256S("0x697905a9b6822eb09a6e3eecb82133cde24f15e5c400368b65bdc9b2cc7943c7"))
                    ( 50, uint256S("0x6a5411cbcbe8d69dd3cc85af05ad7439fc2c02acd8d5861471ea32a1b59ce271"))
                    ( 80000, uint256S("0x0def72391fd1db25297478048a8b1b5feca86061d614146ea8e875d27be1f41f"))
                    (120000, uint256S("0xa6d147731bb021c5365ba44264e7faffd47aaf806861278a4227deac33f78207"))
                    (258805, uint256S("0x74133722e84132005691a21a8092f0c590da7ab5744f3bdf8113089cc6d55051"))
                    //(458580, uint256("0x51fe53e2091ee1f2e8244cf500027a1900e05cd01427a5258dfac8c3d759e7fe"))
                    (564890, uint256S("0x1230d31d9b93651e02c877776e01158496fbac59dd3d898d9b86b76a8e6beb83"))
                    (3533147, uint256S("0x475e42eace5706469310b955d58d0f18376f65e32f5a6aec7348a8cd034df54e")),
                    1574445026, // * UNIX timestamp of last checkpoint block
                    7272545,    // * total number of transactions between genesis and last checkpoint
                                //   (the tx=... number in the SetBestChain debug.log lines)
                    4000.0      // * estimated number of transactions per day after checkpoint
        };
    }
};
static CMainParams mainParams;

/**
 * Testnet
 */
class CTestNetParams : public CChainParams {
public:
    CTestNetParams() {
        strNetworkID = "test";
        consensus.nMaxReorganizationDepth = 60 * 4;
        consensus.nMajorityEnforceBlockUpgrade = 750;
        consensus.nMajorityRejectBlockOutdated = 950;
        consensus.nMajorityWindow = 1000;
        consensus.powLimit = uint256S("00000fffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.posLimit = uint256S("00000fffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.posLimitV2 = uint256S("000000000000ffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nTargetTimespan = 108 * 40;
        consensus.nTargetTimespanNEW = 40;
        consensus.nTargetSpacing = 40;
        consensus.nTargetSpacingNEW = 48;
        consensus.nModifierInterval = 13 * 60;
        consensus.BIP34Height = -1;
        consensus.BIP34Hash = uint256();
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = false;
        consensus.fPoSNoRetargeting = false;
        consensus.nRuleChangeActivationThreshold = 1512; // 75% for testchains
        consensus.nMinerConfirmationWindow = 2016; // nPowTargetTimespan / nPowTargetSpacing
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601; // January 1, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999; // December 31, 2008

        consensus.nDifficultyKGW = 5;
        consensus.nDifficultyDigiShield = 10; 
        consensus.nCoinbaseMaturitySwitch = 10;
        consensus.nCheckPOWFromnTime = 1414964233;
        consensus.nProtocolV3Time = std::numeric_limits<int64_t>::max();

        consensus.nLastPOWBlock = 9999;
        consensus.nStakeTimestampMask = 0xf;
        consensus.nCoinbaseMaturity = 5;
        consensus.nCoinbaseMaturityNEW = 60 * 4;
        consensus.nStakeMinAge = 10 * 60;
        consensus.nStakeMaxAge = 365 * 24 * 60 * 60;

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x00");

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0x00");

        pchMessageStart[0] = 0xfe;
        pchMessageStart[1] = 0xc3;
        pchMessageStart[2] = 0xb9;
        pchMessageStart[3] = 0xde;
        nDefaultPort = 14200;
        nPruneAfterHeight = 1000;

        /*
        genesis = CreateGenesisBlockTestnet(1575659029, 0, 0x1f00ffff, 1, 0);
        consensus.hashGenesisBlock = genesis.GetHash();
        MineNewGenesisBlock(consensus, genesis);
        */

        genesis = CreateGenesisBlock(1420924223, 1729989, 0x1e0ffff0, 1, 420 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0x375a10038dbcbf09dbf211280ffa68f5d0138be4c177bff231e28665b3eb8e91"));
        assert(genesis.hashMerkleRoot == uint256S("0xd5a08606e06eea7eae8a889dbcdcdd84917c10fc8e177ec013a9005305afe53d"));

        vSeeds.push_back(CDNSSeedData("51.15.34.13", "51.15.34.13"));
        vSeeds.push_back(CDNSSeedData("51.15.55.7", "51.15.55.7"));

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,55);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,117);
        base58Prefixes[SECRET_KEY] = std::vector<unsigned char>(1,132);
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x04)(0x88)(0xB2)(0x1E).convert_to_container<std::vector<unsigned char> >();
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x04)(0x88)(0xAD)(0xE4).convert_to_container<std::vector<unsigned char> >();
        cashaddrPrefix = "pottest";

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_test, pnSeed6_test + ARRAYLEN(pnSeed6_test));

        fMiningRequiresPeers = true;
        fDefaultConsistencyChecks = false;
        fRequireStandard = false;
        fMineBlocksOnDemand = false;
        fTestnetToBeDeprecatedFieldRPC = true;

        checkpointData = (CCheckpointData) {
            boost::assign::map_list_of
            ( 100, uint256S("0x6928085ed93ea4ac9478bf0f49f84087b8d17fe244ba579dff1be939223bfa6a")),
            1582398859,
            2000,
            2.0
        };

    }
};
static CTestNetParams testNetParams;

/**
 * Regression test
 */
class CRegTestParams : public CChainParams {
public:
    CRegTestParams() {
        strNetworkID = "regtest";
        consensus.nMaxReorganizationDepth = 5;
        consensus.nMajorityEnforceBlockUpgrade = 51;
        consensus.nMajorityRejectBlockOutdated = 75;
        consensus.nMajorityWindow = 100;
        consensus.powLimit = uint256S("7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.posLimit = uint256S("7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.posLimitV2 = uint256S("7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nTargetTimespan = 40;
        consensus.nTargetTimespanNEW = 40;
        consensus.nTargetSpacing = 40;
        consensus.nTargetSpacingNEW = 48;
        consensus.nModifierInterval = 13 * 60;
        consensus.BIP34Height = -1; // BIP34 has not necessarily activated on regtest
        consensus.BIP34Hash = uint256();
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = true;
        consensus.fPoSNoRetargeting = true;
        consensus.nRuleChangeActivationThreshold = 108; // 75% for regtest
        consensus.nMinerConfirmationWindow = 144; // Faster than normal for regtest (144 instead of 2016)
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601; // January 1, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999; // December 31, 2008

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x00");

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0x00");

        consensus.nDifficultyKGW = 500;
        consensus.nDifficultyDigiShield = 750; 
        consensus.nCoinbaseMaturitySwitch = -1;
        consensus.nCheckPOWFromnTime = 1575660753;
        consensus.nProtocolV3Time = std::numeric_limits<int64_t>::max();

        consensus.nLastPOWBlock = 1000;
        consensus.nStakeTimestampMask = 0xf;
        consensus.nCoinbaseMaturity = 5;
        consensus.nCoinbaseMaturityNEW = 5;
        consensus.nStakeMinAge = 1 * 60;
        consensus.nStakeMaxAge = 365 * 24 * 60 * 60;

        pchMessageStart[0] = 0x70;
        pchMessageStart[1] = 0x35;
        pchMessageStart[2] = 0x22;
        pchMessageStart[3] = 0x06;
        nDefaultPort = 24200;
        nPruneAfterHeight = 100000;

        genesis = CreateGenesisBlockRegtest(1575660752, 1, 0x207fffff, 1, 0);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0x7afed6aa0abc766870909b1c5e1f7e9d7347e3e28351e3d829a7dee888f5d99d"));
        assert(genesis.hashMerkleRoot == uint256S("0x15d1b22c75054f04486de21455b82b20f3d5c5ec68f783399cd4728ac395518d"));

        vFixedSeeds.clear(); //!< Regtest mode doesn't have any fixed seeds.
        vSeeds.clear();      //!< Regtest mode doesn't have any DNS seeds.

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,111);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,196);
        base58Prefixes[SECRET_KEY] = std::vector<unsigned char>(1,239);
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x04)(0x88)(0xB2)(0x1E).convert_to_container<std::vector<unsigned char> >();
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x04)(0x88)(0xAD)(0xE4).convert_to_container<std::vector<unsigned char> >();
        cashaddrPrefix = "potreg";

        fMiningRequiresPeers = false;
        fDefaultConsistencyChecks = true;
        fRequireStandard = false;
        fMineBlocksOnDemand = true;
        fTestnetToBeDeprecatedFieldRPC = false;

    }

    void UpdateBIP9Parameters(Consensus::DeploymentPos d, int64_t nStartTime, int64_t nTimeout)
    {
        consensus.vDeployments[d].nStartTime = nStartTime;
        consensus.vDeployments[d].nTimeout = nTimeout;
    }
};
static CRegTestParams regTestParams;

static CChainParams *pCurrentParams = 0;

const CChainParams &Params() {
    assert(pCurrentParams);
    return *pCurrentParams;
}

CChainParams& Params(const std::string& chain)
{
    if (chain == CBaseChainParams::MAIN)
            return mainParams;
    else if (chain == CBaseChainParams::TESTNET)
            return testNetParams;
    else if (chain == CBaseChainParams::REGTEST)
            return regTestParams;
    else
        throw std::runtime_error(strprintf("%s: Unknown chain %s.", __func__, chain));
}

void SelectParams(const std::string& network)
{
    SelectBaseParams(network);
    pCurrentParams = &Params(network);
}

void UpdateRegtestBIP9Parameters(Consensus::DeploymentPos d, int64_t nStartTime, int64_t nTimeout)
{
    regTestParams.UpdateBIP9Parameters(d, nStartTime, nTimeout);
}
