#include <vector>
#include <QByteArray>
#include <QtGlobal>

#include "LightBird.h"

#define shr(x,n) ((x & 0xFFFFFFFF) >> n)
#define rotr(x,n) (shr(x,n) | (x << (32 - n)))

namespace LightBird
{
    quint32 K[]=
    {   0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
        0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
        0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
        0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
        0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
        0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
        0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
        0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};

    void makeblock(std::vector<quint32>& ret, QByteArray p_msg)
    {
        quint32 cur=0;
        int ind=0;
        for(int i=0; i<p_msg.size(); i++)
        {
            cur = (cur<<8) | (unsigned char)p_msg[i];

            if(i%4==3)
            {
                ret.at(ind++)=cur;
                cur=0;
            }
        }
    }

    class Block
    {
    public:
        std::vector<quint32> msg;

        Block():msg(16, 0) { }


        Block(QByteArray p_msg):msg(16, 0)
        {
            makeblock(msg, p_msg);
        }

    };


    void split(std::vector<Block>& blks, QByteArray& msg)
    {
        for(int i=0; i<msg.size(); i+=64)
            makeblock(blks[i/64].msg, msg.mid(i, 64));
    }


    QByteArray mynum(quint32 x)
    {
        QByteArray ret;
        for(quint32 i=0; i<4; i++)
            ret+=char(0);

        for(quint32 i=4; i>=1; i--)	//big endian machine assumed
        {
            ret += ((char*)(&x))[i-1];
        }
        return ret;
    }

    quint32 ch(quint32 x, quint32 y, quint32 z)
    {
        return (x&y) ^ (~x&z);
    }

    quint32 maj(quint32 x, quint32 y, quint32 z)
    {
        return (x&y) ^ (y&z) ^ (z&x);
    }

    quint32 fn0(quint32 x)
    {
        return rotr(x, 2) ^ rotr(x, 13) ^ rotr(x, 22);
    }

    quint32 fn1(quint32 x)
    {
        return rotr(x, 6) ^ rotr(x, 11) ^ rotr(x, 25);
    }

    quint32 sigma0(quint32 x)
    {
        return rotr(x, 7) ^ rotr(x, 18) ^ shr(x, 3);
    }

    quint32 sigma1(quint32 x)
    {
        return rotr(x, 17) ^ rotr(x, 19) ^ shr(x, 10);
    }

    QByteArray      sha256(QByteArray msg_arr)
    {
        QByteArray  msg;
        msg=msg_arr;
        msg_arr += (char)(1<<7);
        quint32 cur_len = msg.size()*8 + 8;
        quint32 reqd_len = ((msg.size()*8)/512+1) *512;
        quint32 pad_len = reqd_len - cur_len - 64;

        QByteArray pad(pad_len/8, char(0));
        msg_arr += pad;
        QByteArray len_str(mynum(msg.size()*8));
        msg_arr = msg_arr + len_str;

        quint32 num_blk = msg_arr.size()*8/512;
        std::vector<Block> M(num_blk, Block());
        split(M, msg_arr);

        quint32 H[]={0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};

        for(quint32 i=0; i<num_blk; i++)
        {
            std::vector<quint32> W(64, 0);
            for(quint32 t=0; t<16; t++)
            {
                W[t] = M[i].msg[t];
            }


            for(quint32 t=16; t<64; t++)
            {
                W[t] = sigma1(W[t-2]) + W[t-7] + sigma0(W[t-15]) + W[t-16];
            }

            quint32 work[8];
            for(quint32 i=0; i<8; i++)
                work[i] = H[i];

            for(quint32 t=0; t<64; t++)
            {

                quint32 t1, t2;
                t1 = work[7] + fn1(work[4]) + ch(work[4], work[5], work[6]) + K[t] + W[t];
                t2 = fn0(work[0]) + maj(work[0], work[1], work[2]);
                work[7] = work[6];
                work[6] = work[5];
                work[5] = work[4];
                work[4] = work[3] + t1;
                work[3] = work[2];
                work[2] = work[1];
                work[1] = work[0];
                work[0] = t1 + t2;

            }

            for(quint32 i=0; i<8; i++)
            {
                H[i] = work[i] + H[i];
            }
        }
        msg_arr.clear();
        for(quint32 i=0; i<8; i++)
        {
            msg = QByteArray::number(H[i], 16);
            msg_arr += QByteArray().fill('0', 8 - msg.size()) + msg;
        }

        return msg_arr;
    }
}
