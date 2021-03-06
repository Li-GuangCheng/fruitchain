// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2015 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_PRIMITIVES_BLOCK_H
#define BITCOIN_PRIMITIVES_BLOCK_H

#include "hash.h"
#include "primitives/transaction.h"
#include "script/script.h"
#include "serialize.h"
#include "uint256.h"
#include "util.h"
#include "utilstrencodings.h"

/** Nodes collect new transactions into a block, hash them into a hash tree,
 * and scan through nonce values to make the block's hash satisfy proof-of-work
 * requirements.  When they solve the proof-of-work, they broadcast the block
 * to everyone and the block is added to the block chain.  The first transaction
 * in the block is a special one that creates a new coin owned by the creator
 * of the block.
 */
class CBlockHeader
{
public:
    // header
    int32_t nVersion;
    uint256 hashPrevBlock;
    uint256 hashPrevEpisode;
    uint256 hashMerkleRoot;
    uint256 hashFruits;
    uint32_t nTime;
    uint32_t nBits;
    uint32_t nNonce;
    CScript scriptPubKey; //pubkey of creator
    uint8_t nTax;

    CBlockHeader()
    {
        SetNull();
    }

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion)
    {
        READWRITE(this->nVersion);
        READWRITE(hashPrevBlock);
        READWRITE(hashPrevEpisode);
        READWRITE(hashMerkleRoot);
        READWRITE(hashFruits);
        READWRITE(nTime);
        READWRITE(nBits);
        READWRITE(nNonce);
        READWRITE(*(CScriptBase*)(&scriptPubKey));
        READWRITE(nTax);
    }

    void SetNull()
    {
        nVersion = 0;
        hashPrevBlock.SetNull();
        hashPrevEpisode.SetNull();
        hashMerkleRoot.SetNull();
        hashFruits.SetNull();
        nTime = 0;
        nBits = 0;
        nNonce = 0;
        scriptPubKey.clear();
        nTax = 0;
    }

    bool IsNull() const
    {
        return (nBits == 0);
    }

    uint256 GetHash() const;

    int64_t GetBlockTime() const
    {
        return (int64_t)nTime;
    }

    std::string ToString() const
    {
        std::stringstream s;
        s << strprintf("CBlockHeader(hash=%s, ver=0x%08x, hashPrevBlock=%s, hashPrevEpisode=%s, hashMerkleRoot=%s, hashFruits=%s, nTime=%u, nBits=%08x, nNonce=%u, nTax=%u)\n",
                GetHash().ToString(),
                nVersion,
                hashPrevBlock.ToString(),
                hashPrevEpisode.ToString(),
                hashMerkleRoot.ToString(),
                hashFruits.ToString(),
                nTime, nBits, nNonce, nTax);
        return s.str();
    }
};


class CBlock : public CBlockHeader
{
public:
    // network and disk
    std::vector<CTransaction> vtx;
    std::vector<CBlockHeader> vfrt;

    // memory only
    mutable bool fChecked;

    CBlock()
    {
        SetNull();
    }

    CBlock(const CBlockHeader& header)
    {
        SetNull();
        *((CBlockHeader*)this) = header;
    }

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion)
    {
        READWRITE(*(CBlockHeader*)this);
        READWRITE(vtx);
        READWRITE(vfrt);
    }

    void SetNull()
    {
        CBlockHeader::SetNull();
        vtx.clear();
        vfrt.clear();
        fChecked = false;
    }

    CBlockHeader GetBlockHeader() const
    {
        CBlockHeader block;
        block.nVersion = nVersion;
        block.hashPrevBlock = hashPrevBlock;
        block.hashPrevEpisode = hashPrevEpisode;
        block.hashMerkleRoot = hashMerkleRoot;
        block.hashFruits = hashFruits;
        block.nTime = nTime;
        block.nBits = nBits;
        block.nNonce = nNonce;
        block.scriptPubKey = scriptPubKey;
        block.nTax = nTax;
        return block;
    }

    uint256 GetFruitsHash() const
    {
        uint256 hash = uint256();
        std::vector<uint256> vfrtHash;
        for (uint i = 0; i < vfrt.size(); ++i)
            vfrtHash.push_back(vfrt[i].GetHash());
        for (std::vector<uint256>::const_iterator it = vfrtHash.begin(); it != vfrtHash.end(); ++it) {
            hash = Hash(BEGIN(hash), END(hash), BEGIN(*it), END(*it));
        }
        return hash;
    }

    std::string ToString() const;
};

/** Describes a place in the block chain to another node such that if the
 * other node doesn't have the same branch, it can find a recent common trunk.
 * The further back it is, the further before the fork it may be.
 */
struct CBlockLocator {
    std::vector<uint256> vHave;

    CBlockLocator() {}
    CBlockLocator(const std::vector<uint256>& vHaveIn)
    {
        vHave = vHaveIn;
    }

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion)
    {
        if (!(nType & SER_GETHASH))
            READWRITE(nVersion);
        READWRITE(vHave);
    }

    void SetNull()
    {
        vHave.clear();
    }

    bool IsNull() const
    {
        return vHave.empty();
    }
};

/** Compute the consensus-critical block weight (see BIP 141). */
int64_t GetBlockWeight(const CBlock& tx);

#endif // BITCOIN_PRIMITIVES_BLOCK_H
