#include "APFinclude.h"

Op128bit::Op128bit (int lData)
{
        lHigh = 0;
        lLow = lData;
}

Op128bit::Op128bit (unsigned int lData)
{
        lHigh = 0;
        lLow = lData;
}

Op128bit::Op128bit (long lData)
{
        lHigh = 0;
        lLow = lData;
}

Op128bit::Op128bit (unsigned long lData)
{
        lHigh = 0;
        lLow = lData;
}

Op128bit::Op128bit (long long lData)
{
        lHigh = 0;
        lLow = lData;
}

Op128bit::Op128bit (unsigned long long lData)
{
        lHigh = 0;
        lLow = lData;
};

Op128bit::Op128bit (const char* pString)
{
        int i, pos = 0;
        char pBit [128] = {0,};
        char nLen = strlen (pString);
        char pCopyString [128];
        strcpy (pCopyString, pString);
        for (i = 0; i < nLen; i ++) pCopyString [i] -= '0';

        while (pos < 128)
        {
                pBit [pos ++] = pCopyString [nLen - 1] % 2;

                int cnt = 0;
                for (i = 0; i < nLen; i ++)
                {
                        pCopyString [i + 1] += (pCopyString [i] % 2) * 10;
                        cnt += !(pCopyString [i] >>= 1);
                }

                if (cnt == nLen) break;
        }
        strrev (pBit);

        Bit128ToOp128 (this, pBit);
}

bool Op128bit::operator==(const Op128bit& oValue) const
{
        if (lHigh == oValue.lHigh && lLow == oValue.lLow)
                return true;
        else
                return false;
}

bool Op128bit::operator!=(const Op128bit& oValue) const
{
        if (lHigh != oValue.lHigh || lLow != oValue.lLow)
                return true;
        else
                return false;
}

bool Op128bit::operator>(const Op128bit& oValue) const
{
        if (lHigh > oValue.lHigh)
                return true;
        else if (lHigh < oValue.lHigh)
                return false;
        else
        {
                if (lLow > oValue.lLow)
                        return true;
                else
                        return false;
        }
}

bool Op128bit::operator<(const Op128bit& oValue) const
{
        if (lHigh < oValue.lHigh)
                return true;
        else if (lHigh > oValue.lHigh)
                return false;
        else
        {
                if (lLow < oValue.lLow)
                        return true;
                else
                        return false;
        }
}

bool Op128bit::operator>=(const Op128bit& oValue) const
{
        if (lHigh > oValue.lHigh)
                return true;
        else if (lHigh < oValue.lHigh)
                return false;
        else
        {
                if (lLow >= oValue.lLow)
                        return true;
                else
                        return false;
        }
}

bool Op128bit::operator<=(const Op128bit& oValue) const
{
        if (lHigh < oValue.lHigh)
                return true;
        else if (lHigh > oValue.lHigh)
                return false;
        else
        {
                if (lLow <= oValue.lLow)
                        return true;
                else
                        return false;
        }
}

Op128bit Op128bit::operator<<(unsigned int nBit) const
{
        Op128bit oTmp (*this);

        unsigned long long tmp = lLow;
        for (unsigned int i = 0; i < 64 - nBit; i ++) tmp >>= 1;

        oTmp.lHigh = (lHigh << nBit) + tmp;
        oTmp.lLow = (lLow << nBit);
        return oTmp;
}

Op128bit Op128bit::operator>>(unsigned int nBit) const
{
        Op128bit oTmp (*this);

        unsigned long long tmp = lHigh;
        for (unsigned int i = 0; i < 64 - nBit; i ++) tmp <<= 1;

        oTmp.lLow = tmp + (lLow >> nBit);
        oTmp.lHigh = (lHigh >> nBit);
        return oTmp;
}

Op128bit Op128bit::operator+(const Op128bit& oValue) const
{
        int i;
        Op128bit oTmp;
        char d1 [129] = {0,};
        char d2 [128] = {0,};
        Op128ToBit128 (d1, lHigh, lLow);
        Op128ToBit128 (d2, oValue.lHigh, oValue.lLow);

        for (i = 0; i < 128; i ++)
        {
                d1 [i] += d2 [i];
                d1 [i + 1] += (d1 [i]>>1);
                d1 [i] %= 2;
        }
        Bit128ToOp128 (&oTmp, d1);

        return oTmp;
}

Op128bit Op128bit::operator-(const Op128bit& oValue) const
{
        int i;
        Op128bit oTmp;
        char d1 [129] = {0,};
        char d2 [128] = {0,};
        Op128ToBit128 (d1, lHigh, lLow);
        Op128ToBit128 (d2, oValue.lHigh, oValue.lLow);
        for (i = 0; i < 128; i ++)
                d2 [i] = !d2 [i];

        d1 [0] += 1;
        for (i = 0; i < 128; i ++)
        {
                d1 [i] += d2 [i];
                d1 [i + 1] += (d1 [i]>>1);
                d1 [i] %= 2;
        }
        Bit128ToOp128 (&oTmp, d1);

        return oTmp;
}

Op128bit Op128bit::operator*(const Op128bit& oValue) const
{
        Op128bit oTmp;
        unsigned int data1 [4] = {0,};
        unsigned int data2 [4] = {0,};
        unsigned int data3 [4] = {0,};
        data1 [0] = (unsigned int)(lLow);
        data1 [1] = (unsigned int)(lLow >> 32);
        data1 [2] = (unsigned int)(lHigh);
        data1 [3] = (unsigned int)(lHigh >> 32);
        data2 [0] = (unsigned int)(oValue.lLow);
        data2 [1] = (unsigned int)(oValue.lLow >> 32);
        data2 [2] = (unsigned int)(oValue.lHigh);
        data2 [3] = (unsigned int)(oValue.lHigh >> 32);

        unsigned long long res;
        for (int i = 0; i < 4; i ++)
        {
                for (int j = 0; j < 4; j ++)
                {
                        res = (unsigned long long)(data1 [i]) * data2 [j];
                        data3 [i + j] += (unsigned int)(res);
                        if (i + j + 1 < 4)
                                data3 [i + j + 1] += (unsigned int)(res >> 32);
                }
        }

        oTmp.lLow = ((unsigned long long)data3 [1] << 32) + (unsigned long long)data3 [0];
        oTmp.lHigh = ((unsigned long long)data3 [3] << 32) + (unsigned long long)data3 [2];

        return oTmp;
}

Op128bit Op128bit::operator/(const Op128bit& oValue) const
{
        Op128bit oTmp;
        Op128bit oRemainder (*this);
        Op128bit oDivisor;
        unsigned int data1 [4] = {0,};
        unsigned int data2 [4] = {0,};
        unsigned int data3 [4] = {0,};
        data1 [0] = (unsigned int)(oRemainder.lLow);
        data1 [1] = (unsigned int)(oRemainder.lLow >> 32);
        data1 [2] = (unsigned int)(oRemainder.lHigh);
        data1 [3] = (unsigned int)(oRemainder.lHigh >> 32);
        data2 [0] = (unsigned int)(oValue.lLow);
        data2 [1] = (unsigned int)(oValue.lLow >> 32);
        data2 [2] = (unsigned int)(oValue.lHigh);
        data2 [3] = (unsigned int)(oValue.lHigh >> 32);

        unsigned int ns = Op128Usebits (data1);
        unsigned int ms = Op128Usebits (data2);
        if (ns < ms)
                return oTmp;
        if (ms == 0)
                return oTmp;

        oDivisor = oValue << (ns - ms);
        for (int i = ns - ms; i >= 0; i --)
        {
                if (oRemainder >= oDivisor)
                {
                        oRemainder = oRemainder - oDivisor;
                        if ( (i >> 5) < 4)
                                data3 [i >> 5] |= 1 << (i & 31);
                }
                oDivisor = oDivisor >> 1;
        }

        oTmp.lLow = ((unsigned long long)data3 [1] << 32) + (unsigned long long)data3 [0];
        oTmp.lHigh = ((unsigned long long)data3 [3] << 32) + (unsigned long long)data3 [2];

        return oTmp;
}

Op128bit Op128bit::operator%(const Op128bit& oValue) const
{
        Op128bit oRemainder (*this);
        Op128bit oDivisor;
        unsigned int data1 [4] = {0,};
        unsigned int data2 [4] = {0,};
        data1 [0] = (unsigned int)(oRemainder.lLow);
        data1 [1] = (unsigned int)(oRemainder.lLow >> 32);
        data1 [2] = (unsigned int)(oRemainder.lHigh);
        data1 [3] = (unsigned int)(oRemainder.lHigh >> 32);
        data2 [0] = (unsigned int)(oValue.lLow);
        data2 [1] = (unsigned int)(oValue.lLow >> 32);
        data2 [2] = (unsigned int)(oValue.lHigh);
        data2 [3] = (unsigned int)(oValue.lHigh >> 32);

        unsigned int ns = Op128Usebits (data1);
        unsigned int ms = Op128Usebits (data2);
        if (ns < ms)
                return oRemainder;
        if (ms == 0)
                return oRemainder;

        oDivisor = oValue << (ns - ms);
        for (int i = ns - ms; i >= 0; i --)
        {
                if (oRemainder >= oDivisor)
                        oRemainder = oRemainder - oDivisor;
                oDivisor = oDivisor >> 1;
        }

        return oRemainder;
}

Op128bit Op128bit::operator&(const Op128bit& oValue) const
{
        Op128bit oTmp;

        oTmp.lHigh = lHigh & oValue.lHigh;
        oTmp.lLow = lLow & oValue.lLow;

        return oTmp;
}

Op128bit Op128bit::operator|(const Op128bit& oValue) const
{
        Op128bit oTmp;

        oTmp.lHigh = lHigh | oValue.lHigh;
        oTmp.lLow = lLow | oValue.lLow;

        return oTmp;
}

Op128bit Op128bit::operator^(const Op128bit& oValue) const
{
        Op128bit oTmp;

        oTmp.lHigh = lHigh ^ oValue.lHigh;
        oTmp.lLow = lLow ^ oValue.lLow;

        return oTmp;
}

Op128bit Op128bit::operator!(void) const
{
        Op128bit oTmp;

        oTmp.lHigh = !lHigh;
        oTmp.lLow = !lLow;

        return oTmp;
}

Op128bit Op128bit::operator<<=(const unsigned int& nValue)
{
        return (*this = *this << nValue);
}

Op128bit Op128bit::operator>>=(const unsigned int& nValue)
{
        return (*this = *this >> nValue);
}

Op128bit Op128bit::operator+=(const Op128bit& oValue)
{
        return (*this = *this + oValue);
}

Op128bit Op128bit::operator-=(const Op128bit& oValue)
{
        return (*this = *this - oValue);
}

Op128bit Op128bit::operator*=(const Op128bit& oValue)
{
        return (*this = *this * oValue);
}

Op128bit Op128bit::operator/=(const Op128bit& oValue)
{
        return (*this = *this / oValue);
}

unsigned long long Op128bit::ConvertToInt64 (void)
{
        return lLow;
}

unsigned long Op128bit::ConvertToInt32 (void)
{
        return lLow & 0xFFFFFFFF;
}

void Op128ToBit128(char* pBit, unsigned long long nHighValue, unsigned long long nLowValue)
{
        int i = 0;
        while (nLowValue != 0)
        {
                pBit [i ++] = nLowValue % 2;
                nLowValue >>= 1;
        }
        i = 64;
        while (nHighValue != 0)
        {
                pBit [i ++] = nHighValue % 2;
                nHighValue >>= 1;
        }
}

void Bit128ToOp128 (Op128bit* pData, char* pBit)
{
        pData->lHigh = 0;
        pData->lLow = 0;
        for (int i = 63; i >= 0; i --)
        {
                pData->lLow <<= 1;
                if (pBit [i])
                        pData->lLow += 1;
        }
        for (int i = 127; i >= 64; i --)
        {
                pData->lHigh <<= 1;
                if (pBit [i])
                        pData->lHigh += 1;
        }
}

unsigned int Op128Usebits (const unsigned int* src)
{
        for (int i = 3; i >= 0; i --)
        {
                int n = src [i];
                if (n == 0)
                        continue;

                unsigned int b = 0;
                if (n >> 16)
                {
                        n >>= 16;
                        b += 16;
                }
                if (n >> 8)
                {
                        n >>= 8;
                        b += 8;
                }
                if (n >> 4)
                {
                        n >>= 4;
                        b += 4;
                }
                if (n >> 2)
                {
                        n >>= 2;
                        b += 2;
                }
                if (n >> 1)
                {
                        n >>= 1;
                        b += 1;
                }
                return i * 32 + b;
        }

        return -1;
}
