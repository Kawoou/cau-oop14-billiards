#ifndef __OPERATOR_128BIT__
#define __OPERATOR_128BIT__

#pragma once

class Op128bit
{
public:
        unsigned long long lHigh;
        unsigned long long lLow;

        Op128bit ()
        {
                lHigh = lLow = 0;
        };
        Op128bit (const Op128bit& v)
        {
                lHigh = v.lHigh;
                lLow = v.lLow;
        };
        Op128bit (unsigned long long a, unsigned long long b)
        {
                lHigh = a;
                lLow = b;
        };

        Op128bit (int lData);
        Op128bit (unsigned int lData);
        Op128bit (long lData);
        Op128bit (unsigned long lData);
        Op128bit (long long lData);
        Op128bit (unsigned long long lData);
        Op128bit (const char* pString);

        bool operator==(const Op128bit& oValue) const;
        bool operator!=(const Op128bit& oValue) const;
        bool operator>(const Op128bit& oValue) const;
        bool operator<(const Op128bit& oValue) const;
        bool operator>=(const Op128bit& oValue) const;
        bool operator<=(const Op128bit& oValue) const;

        Op128bit operator<<(unsigned int nBit) const;
        Op128bit operator>>(unsigned int nBit) const;
        Op128bit operator+(const Op128bit& oValue) const;
        Op128bit operator-(const Op128bit& oValue) const;
        Op128bit operator*(const Op128bit& oValue) const;
        Op128bit operator/(const Op128bit& oValue) const;
        Op128bit operator%(const Op128bit& oValue) const;

        Op128bit operator&(const Op128bit& oValue) const;
        Op128bit operator|(const Op128bit& oValue) const;
        Op128bit operator^(const Op128bit& oValue) const;
        Op128bit operator!(void) const;

        Op128bit operator<<=(const unsigned int& nValue);
        Op128bit operator>>=(const unsigned int& nValue);
        Op128bit operator+=(const Op128bit& oValue);
        Op128bit operator-=(const Op128bit& oValue);
        Op128bit operator*=(const Op128bit& oValue);
        Op128bit operator/=(const Op128bit& oValue);

        unsigned long long ConvertToInt64 (void);
        unsigned long ConvertToInt32 (void);
};

void Op128ToBit128 (char* pBit, unsigned long long nHighValue, unsigned long long nLowValue);
void Bit128ToOp128 (Op128bit* pData, char* pBit);
unsigned int Op128Usebits (const unsigned int* src);

#endif
