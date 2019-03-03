// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Copyright (c) 2014-2015 The Dash developers
// Copyright (c) 2015-2018 The PIVX developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "libzerocoin/Params.h"
#include "chainparams.h"
#include "random.h"
#include "util.h"
#include "utilstrencodings.h"

#include <assert.h>

#include <boost/assign/list_of.hpp>

using namespace std;
using namespace boost::assign;

struct SeedSpec6 {
    uint8_t addr[16];
    uint16_t port;
};

#include "chainparamsseeds.h"

/**
 * Main network
 */

//! Convert the pnSeeds6 array into usable address objects.
static void convertSeed6(std::vector<CAddress>& vSeedsOut, const SeedSpec6* data, unsigned int count)
{
    // It'll only connect to one or two seed nodes because once it connects,
    // it'll get a pile of addresses with newer timestamps.
    // Seed nodes are given a random 'last seen time' of between one and two
    // weeks ago.
    const int64_t nOneWeek = 7 * 24 * 60 * 60;
    for (unsigned int i = 0; i < count; i++) {
        struct in6_addr ip;
        memcpy(&ip, data[i].addr, sizeof(ip));
        CAddress addr(CService(ip, data[i].port));
        addr.nTime = GetTime() - GetRand(nOneWeek) - nOneWeek;
        vSeedsOut.push_back(addr);
    }
}

//   What makes a good checkpoint block?
// + Is surrounded by blocks with reasonable timestamps
//   (no blocks before with a timestamp after, none after with
//    timestamp before)
// + Contains no strange transactions
static Checkpoints::MapCheckpoints mapCheckpoints =
    boost::assign::map_list_of
    ( 15000, uint256("0xca3548f27e851f563008cfd79b3f97b555002ade6d6d1abf82af5161b3e5b0df"))
    ( 55000, uint256("0xb12293afc8d69f285cf0f7c1767b55d27c571c499ac91bdb47a5bccfa76b63b8"))
    (110000, uint256("0x92f5e226fb7bcc58f9933fa1c4836b5e2bf029bdb8e9eae366d80b14017841bc"))
    (300500, uint256("0x09d9bd6b9002bf66527e098f0e620a6f59c6a3b244bc0bc8f4c4879e19b8fbfa"))
    (345000, uint256("0x251f44f94e369ebebc51304ce1a7b367ecad8025e822a5a28407085915542a47"))
    (427520, uint256("0xbcb1998feb84a6741beba7b11eb28fc496f3eacc51e110b3f7b03749b5c5b37e"));
static const Checkpoints::CCheckpointData data = {
    &mapCheckpoints,
    1524214096, // * UNIX timestamp of last checkpoint block
    863335,     // * total number of transactions between genesis and last checkpoint
                //   (the tx=... number in the SetBestChain debug.log lines)
    1920        // * estimated number of transactions per day after checkpoint
};

static Checkpoints::MapCheckpoints mapCheckpointsTestnet =
    boost::assign::map_list_of
    (100, uint256("0x000008a085bde61a7b4aacb02d0d634d4bb77c37354e98111f3f3d72e9440198")); // last PoW block on testnet
static const Checkpoints::CCheckpointData dataTestnet = {
    &mapCheckpointsTestnet,
    1551471258,
    122,
    1920};

static Checkpoints::MapCheckpoints mapCheckpointsRegtest =
    boost::assign::map_list_of(0, uint256("0x001"));
static const Checkpoints::CCheckpointData dataRegtest = {
    &mapCheckpointsRegtest,
    1512432000,
    0,
    0};

libzerocoin::ZerocoinParams* CChainParams::Zerocoin_Params(bool useModulusV1) const
{
    assert(this);
    static CBigNum bnHexModulus = 0;
    if (!bnHexModulus)
        bnHexModulus.SetHex(zerocoinModulus);
    static libzerocoin::ZerocoinParams ZCParamsHex = libzerocoin::ZerocoinParams(bnHexModulus);
    static CBigNum bnDecModulus = 0;
    if (!bnDecModulus)
        bnDecModulus.SetDec(zerocoinModulus);
    static libzerocoin::ZerocoinParams ZCParamsDec = libzerocoin::ZerocoinParams(bnDecModulus);

    if (useModulusV1)
        return &ZCParamsHex;

    return &ZCParamsDec;
}

class CMainParams : public CChainParams
{
public:
    CMainParams()
    {
        networkID = CBaseChainParams::MAIN;
        strNetworkID = "main";
        /**
         * The message start string is designed to be unlikely to occur in normal data.
         * The characters are rarely used upper ASCII, not valid as UTF-8, and produce
         * a large 4-byte int at any alignment.
         */
        pchMessageStart[0] = 0xdf;
        pchMessageStart[1] = 0x1c;
        pchMessageStart[2] = 0x13;
        pchMessageStart[3] = 0xf8;
        vAlertPubKey = ParseHex("04b5ffc3286e618001604b5e16eefc30812c7d0b2dc91af9ee470c4605a9183d162c912bb437341270f00637432d2af3e7c01bcabcbdb8c085d56d97a08f355ce4");
        nDefaultPort = 40420;
        bnProofOfWorkLimit = ~uint256(0) >> 20; // GROW starting difficulty is 1 / 2^12
        bnProofOfStakeLimit = ~uint256(0) >> 48;
        nMaxReorganizationDepth = 500;
        nEnforceBlockUpgradeMajority = 5400; // 75%
        nRejectBlockOutdatedMajority = 6840; // 95%
        nToCheckBlockUpgradeMajority = 7200; // Approximate expected amount of blocks in 7 days (960*7.5)
        nMinerThreads = 0;
        nTargetTimespan = 20 * 90;   // GROW: 30 minutes
        nTargetSpacing = 1 * 90;     // GROW: 90 seconds
        nMaturity = 60;              // GROW: 60 blocks
        nModifierInterval = 15 * 60; // GROW: 15 minutes
        nStakeMinAge = 12 * 60 * 60; // GROW: 12 hours
        nMasternodeCountDrift = 20;
        nMaxMoneyOut = 200000000 * COIN;

        /** Height or Time Based Activations **/
        nLastPOWBlock = 750;
        nZerocoinStartHeight = std::numeric_limits<int>::max();
        nZerocoinStartTime = std::numeric_limits<int>::max();
        nNewRewardStructureHeight = std::numeric_limits<int>::max();
        nNewRewardStructureTime = std::numeric_limits<int>::max();

        /**
         * Build the genesis block. Note that the output of the genesis coinbase cannot
         * be spent as it did not originally exist in the database.
         *
         * CBlock(hash=00000ffd590b14, ver=1, hashPrevBlock=00000000000000, hashMerkleRoot=e0028e, nTime=1390095618, nBits=1e0ffff0, nNonce=28917698, vtx=1)
         *   CTransaction(hash=e0028e, ver=1, vin.size=1, vout.size=1, nLockTime=0)
         *     CTxIn(COutPoint(000000, -1), coinbase 04ffff001d01044c5957697265642030392f4a616e2f3230313420546865204772616e64204578706572696d656e7420476f6573204c6976653a204f76657273746f636b2e636f6d204973204e6f7720416363657074696e6720426974636f696e73)
         *     CTxOut(nValue=50.00000000, scriptPubKey=0xA9037BAC7050C479B121CF)
         *   vMerkleTree: e0028e
         */
        const char* pszTimestamp = "Dope is great for the World and more";
        CMutableTransaction txNew;
        txNew.vin.resize(1);
        txNew.vout.resize(1);
        txNew.vin[0].scriptSig = CScript() << 0 << CScriptNum(42) << vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
        txNew.vout[0].SetEmpty();
        txNew.nTime = 1480636800;
        genesis.vtx.push_back(txNew);
        genesis.hashPrevBlock = 0;
        genesis.hashMerkleRoot = genesis.BuildMerkleTree();
        genesis.nVersion = 1;
        genesis.nTime = 1480636800;
        genesis.nBits = 0x1e0fffff;
        genesis.nNonce = 499515;

        hashGenesisBlock = genesis.GetHash();
        assert(hashGenesisBlock == uint256("0x2acfa8ef95f8014d38217cdcecd3850e081a86976bb1bb8497f519fc1dcac3bc"));
        assert(genesis.hashMerkleRoot == uint256("0x0e58d4b3215bb1889d5a027d40269a167b93c68e9ae28961de4717558be92d38"));

        vSeeds.push_back(CDNSSeedData("dopecoin.com", "dnsseed.dopecoin.com"));
        vSeeds.push_back(CDNSSeedData("188.166.89.189", "188.166.89.189"));
        vSeeds.push_back(CDNSSeedData("37.120.190.76", "37.120.190.76"));
        vSeeds.push_back(CDNSSeedData("37.120.186.85", "37.120.186.85"));
        vSeeds.push_back(CDNSSeedData("188.68.52.172", "188.68.52.172"));

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1, 30);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1, 5);
        base58Prefixes[SECRET_KEY] = std::vector<unsigned char>(1, 158);
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x04)(0x88)(0xB2)(0x1E).convert_to_container<std::vector<unsigned char> >();
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x04)(0x88)(0xAD)(0xE4).convert_to_container<std::vector<unsigned char> >();
        // 	BIP44 coin type is from https://github.com/satoshilabs/slips/blob/master/slip-0044.md
        base58Prefixes[EXT_COIN_TYPE] = boost::assign::list_of(0x80)(0x00)(0x00)(0x35).convert_to_container<std::vector<unsigned char> >();

        convertSeed6(vFixedSeeds, pnSeed6_main, ARRAYLEN(pnSeed6_main));

        fMiningRequiresPeers = true;
        fAllowMinDifficultyBlocks = false;
        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        fMineBlocksOnDemand = false;
        fSkipProofOfWorkCheck = false;
        fTestnetToBeDeprecatedFieldRPC = false;
        fHeadersFirstSyncingActive = false;

        nPoolMaxTransactions = 3;
        strSporkKey = "040EE37D745991B433EC58DE821E1B6EFAC7D80DA9CFDCF42F97C4224DC468BF1ED89D4898C48EE9822A485D19016051BCC6A85989CEAB9D8A1FA129E1015D5294";
        strObfuscationPoolDummyAddress = "D7qLCPEFsCLNGcLUBNHazPEuvn5xxWUtHd";
        nStartMasternodePayments = nNewRewardStructureTime;

        /** Zerocoin */
        zerocoinModulus = "25195908475657893494027183240048398571429282126204032027777137836043662020707595556264018525880784"
            "4069182906412495150821892985591491761845028084891200728449926873928072877767359714183472702618963750149718246911"
            "6507761337985909570009733045974880842840179742910064245869181719511874612151517265463228221686998754918242243363"
            "7259085141865462043576798423387184774447920739934236584823824281198163815010674810451660377306056201619676256133"
            "8441436038339044149526344321901146575444541784240209246165157233507787077498171257724679629263863563732899121548"
            "31438167899885040445364023527381951378636564391212010397122822120720357";
        nMaxZerocoinSpendsPerTransaction = 7; // Assume about 20kb each
        nMinZerocoinMintFee = 1 * CENT; //high fee required for zerocoin mints
        nMintRequiredConfirmations = 20; //the maximum amount of confirmations until accumulated in 19
        nRequiredAccumulation = 1;
        nDefaultSecurityLevel = 100; //full security level for accumulators
        nZerocoinHeaderVersion = 5; //Block headers must be this version once zerocoin is active
        nZerocoinRequiredStakeDepth = 200; //The required confirmations for a zPIV/zGROW to be stakable

        nBudget_Fee_Confirmations = 6; // Number of confirmations for the finalization fee
    }

    const Checkpoints::CCheckpointData& Checkpoints() const
    {
        return data;
    }
};
static CMainParams mainParams;

/**
 * Testnet (v3)
 */
class CTestNetParams : public CMainParams
{
public:
    CTestNetParams()
    {
        networkID = CBaseChainParams::TESTNET;
        strNetworkID = "test";
        pchMessageStart[0] = 0xcd;
        pchMessageStart[1] = 0xf2;
        pchMessageStart[2] = 0xc0;
        pchMessageStart[3] = 0xef;
        vAlertPubKey = ParseHex("04530bafe00460bf4f91ef8579684dafd9ef312e1c3d696a0a233c79019a11273a7419a4df97737fdab2cb01bd78fa923c136d54e71bb7ab272162a99047f11fb6");
        nDefaultPort = 30420;
        nEnforceBlockUpgradeMajority = 2880; // 75%
        nRejectBlockOutdatedMajority = 3648; // 95%
        nToCheckBlockUpgradeMajority = 3840; // 4 days
        nMinerThreads = 0;
        nTargetTimespan = 20 * 90; // GROW: 30 minutes
        nTargetSpacing = 1 * 90;   // GROW: 90 seconds
        nMaturity = 10;
        nModifierInterval = 15 * 60;
        nStakeMinAge = 1 * 60 * 60;
        nMasternodeCountDrift = 4;
        nMaxMoneyOut = 200000000 * COIN;

        nLastPOWBlock = 100;
        nZerocoinStartHeight = std::numeric_limits<int>::max();
        nZerocoinStartTime = std::numeric_limits<int>::max();
        nNewRewardStructureHeight = std::numeric_limits<int>::max();
        nNewRewardStructureTime = std::numeric_limits<int>::max();

        //! Modify the testnet genesis block so the timestamp is valid for a later start.
        genesis.nNonce = 499515;

        hashGenesisBlock = genesis.GetHash();
        assert(hashGenesisBlock == uint256("0x2acfa8ef95f8014d38217cdcecd3850e081a86976bb1bb8497f519fc1dcac3bc"));

        vSeeds.push_back(CDNSSeedData("51.15.34.13", "51.15.34.13"));

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1, 111);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1, 196);
        base58Prefixes[SECRET_KEY] = std::vector<unsigned char>(1, 239);     // Testnet private keys start with '9' or 'c' (Bitcoin defaults)
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x04)(0x35)(0x87)(0xCF).convert_to_container<std::vector<unsigned char> >();
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x04)(0x35)(0x83)(0x94).convert_to_container<std::vector<unsigned char> >();
        // Testnet BIP44 coin type is '1' (All coin's testnet default)
        base58Prefixes[EXT_COIN_TYPE] = boost::assign::list_of(0x80)(0x00)(0x00)(0x01).convert_to_container<std::vector<unsigned char> >();

        convertSeed6(vFixedSeeds, pnSeed6_test, ARRAYLEN(pnSeed6_test));

        fMiningRequiresPeers = true;
        fAllowMinDifficultyBlocks = true;
        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        fMineBlocksOnDemand = false;
        fTestnetToBeDeprecatedFieldRPC = true;

        nPoolMaxTransactions = 2;
        strSporkKey = "0456DEB2969D41F1E9C1CA51E6B1DE8AD2C5CA1F4C24BF64B778F6855458DE0220532F815D54FA38FE2085F644B8C072A191E13747B4A880D24FC8852808FA897A";
        strObfuscationPoolDummyAddress = "mvnkQw3iGLr3vr6SokPp7HNGdfXQtCUk9E";
        nStartMasternodePayments = nNewRewardStructureTime;

        nBudget_Fee_Confirmations = 3; // Number of confirmations for the finalization fee. We have to make this very short
                                       // here because we only have a 8 block finalization window on testnet
    }
    const Checkpoints::CCheckpointData& Checkpoints() const
    {
        return dataTestnet;
    }
};
static CTestNetParams testNetParams;

/**
 * Regression test
 */
class CRegTestParams : public CTestNetParams
{
public:
    CRegTestParams()
    {
        networkID = CBaseChainParams::REGTEST;
        strNetworkID = "regtest";
        pchMessageStart[0] = 0xfa;
        pchMessageStart[1] = 0xbf;
        pchMessageStart[2] = 0xb5;
        pchMessageStart[3] = 0xda;
        nDefaultPort = 20420;
        nEnforceBlockUpgradeMajority = 750;
        nRejectBlockOutdatedMajority = 950;
        nToCheckBlockUpgradeMajority = 1000;
        nMinerThreads = 1;
        nTargetTimespan = 20 * 90; // GROW: 30 minutes
        nTargetSpacing = 1 * 90;   // GROW: 90 seconds
        bnProofOfWorkLimit = ~uint256(0) >> 1;
        nMaturity = 10;
        nModifierInterval = 15 * 60;
        nStakeMinAge = 1 * 60 * 60;
        nMasternodeCountDrift = 4;
        nMaxMoneyOut = 200000000 * COIN;

        nLastPOWBlock = 100;
        nZerocoinStartHeight = 300;
        nZerocoinStartTime = 1501776000;
        nNewRewardStructureHeight = nZerocoinStartHeight;
        nNewRewardStructureTime = nZerocoinStartTime;

        CMutableTransaction txNew;
        txNew.nTime = 1550741119;
        genesis.nTime = 1550741119;
        genesis.nBits = 0x1e0fffff;
        genesis.nNonce = 1;

        hashGenesisBlock = genesis.GetHash();
        assert(hashGenesisBlock == uint256("0xc4e5324385e4f591d4c2f173aba2d59e2d5bf1aa06f3f7e6b7fabca5fc9d51f0"));

        vFixedSeeds.clear(); //! Regtest mode doesn't have any fixed seeds.
        vSeeds.clear();      //! Regtest mode doesn't have any DNS seeds.

        fMiningRequiresPeers = false;
        fAllowMinDifficultyBlocks = true;
        fDefaultConsistencyChecks = true;
        fRequireStandard = false;
        fMineBlocksOnDemand = true;
        fSkipProofOfWorkCheck = true;
        fTestnetToBeDeprecatedFieldRPC = false;
    }
    const Checkpoints::CCheckpointData& Checkpoints() const
    {
        return dataRegtest;
    }
};
static CRegTestParams regTestParams;

/**
 * Unit test
 */
class CUnitTestParams : public CMainParams, public CModifiableParams
{
public:
    CUnitTestParams()
    {
        networkID = CBaseChainParams::UNITTEST;
        strNetworkID = "unittest";
        nDefaultPort = 10420;
        vFixedSeeds.clear(); //! Unit test mode doesn't have any fixed seeds.
        vSeeds.clear();      //! Unit test mode doesn't have any DNS seeds.

        fMiningRequiresPeers = false;
        fDefaultConsistencyChecks = true;
        fAllowMinDifficultyBlocks = false;
        fMineBlocksOnDemand = true;
    }

    const Checkpoints::CCheckpointData& Checkpoints() const
    {
        // UnitTest share the same checkpoints as MAIN
        return data;
    }

    //! Published setters to allow changing values in unit test cases
    virtual void setEnforceBlockUpgradeMajority(int anEnforceBlockUpgradeMajority) { nEnforceBlockUpgradeMajority = anEnforceBlockUpgradeMajority; }
    virtual void setRejectBlockOutdatedMajority(int anRejectBlockOutdatedMajority) { nRejectBlockOutdatedMajority = anRejectBlockOutdatedMajority; }
    virtual void setToCheckBlockUpgradeMajority(int anToCheckBlockUpgradeMajority) { nToCheckBlockUpgradeMajority = anToCheckBlockUpgradeMajority; }
    virtual void setDefaultConsistencyChecks(bool afDefaultConsistencyChecks) { fDefaultConsistencyChecks = afDefaultConsistencyChecks; }
    virtual void setAllowMinDifficultyBlocks(bool afAllowMinDifficultyBlocks) { fAllowMinDifficultyBlocks = afAllowMinDifficultyBlocks; }
    virtual void setSkipProofOfWorkCheck(bool afSkipProofOfWorkCheck) { fSkipProofOfWorkCheck = afSkipProofOfWorkCheck; }
};
static CUnitTestParams unitTestParams;


static CChainParams* pCurrentParams = 0;

CModifiableParams* ModifiableParams()
{
    assert(pCurrentParams);
    assert(pCurrentParams == &unitTestParams);
    return (CModifiableParams*)&unitTestParams;
}

const CChainParams& Params()
{
    assert(pCurrentParams);
    return *pCurrentParams;
}

CChainParams& Params(CBaseChainParams::Network network)
{
    switch (network) {
    case CBaseChainParams::MAIN:
        return mainParams;
    case CBaseChainParams::TESTNET:
        return testNetParams;
    case CBaseChainParams::REGTEST:
        return regTestParams;
    case CBaseChainParams::UNITTEST:
        return unitTestParams;
    default:
        assert(false && "Unimplemented network");
        return mainParams;
    }
}

void SelectParams(CBaseChainParams::Network network)
{
    SelectBaseParams(network);
    pCurrentParams = &Params(network);
}

bool SelectParamsFromCommandLine()
{
    CBaseChainParams::Network network = NetworkIdFromCommandLine();
    if (network == CBaseChainParams::MAX_NETWORK_TYPES)
        return false;

    SelectParams(network);
    return true;
}
