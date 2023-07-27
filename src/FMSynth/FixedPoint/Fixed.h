// This implementation uses some brilliant techniques from Pharap's fixed point library. You 
// can find the source code of the library here: https://github.com/Pharap/FixedPointsArduino

#pragma once

#include <cstdint>

namespace FixedPoint {

template<unsigned FractionBits>
class Fixed {
    
    public:
    
        using IntegerType = std::int32_t;
        using FractionType = std::int32_t;
        using InternalType = std::int32_t;
        using PrecisionType = std::int64_t;
        
        constexpr static const InternalType FRACTION_MASK = (1 << FractionBits) - 1;

    public:
    
        static constexpr Fixed Pi = Fixed(3.1415926536);
        
        constexpr Fixed(): _value(0) {}
        
        explicit constexpr Fixed(const int& val) : _value(val << FractionBits) {}
        explicit constexpr Fixed(const long int& val) : _value(val << FractionBits) {}
        explicit constexpr Fixed(const float& val) : _value(val * (1 << FractionBits)) {}
        explicit constexpr Fixed(const double& val) : _value(val * (1 << FractionBits)) {}
        
        constexpr Fixed(const IntegerType& integer, const FractionType& fraction) : _value((integer << FractionBits) + fraction) {}
        
        // Named constructors:
        static constexpr Fixed fromInternal(const InternalType& value) { return Fixed(InternalTypeWrapper(value)); }
        
        constexpr IntegerType integer() const { return (_value >= 0) ? _value >> FractionBits : -(-_value >> FractionBits); }
        constexpr Fixed fraction() const { return fromInternal((_value >= 0) ? _value & FRACTION_MASK : (_value | ~FRACTION_MASK)); }
        constexpr InternalType internal() const { return _value; }
        
        // Number of fractional bits
        static constexpr std::size_t fractionalBits() { return FractionBits; }
        
        // Typecasts
        explicit constexpr operator IntegerType() const { return integer(); }
        explicit constexpr operator float() const { return static_cast<float>(_value) / (1 << FractionBits); }
        
        // Copy constructor
        constexpr Fixed(const Fixed& other) : _value(other._value) {}
        
        // Minus operator
        constexpr Fixed operator -() const { return fromInternal(-_value); }
        
        // Increment and decrement operators
        Fixed& operator ++() {
            ++_value;
            return *this;
        }
        Fixed& operator --() {
            --_value;
            return *this;
        }
        
        // Assignment operators
        Fixed& operator =(const Fixed& other) {
            _value = other._value;
            return *this;
        }
        Fixed& operator +=(const Fixed& other) {
            _value += other._value;
            return *this;
        }
        Fixed& operator -=(const Fixed& other) {
            _value -= other._value;
            return *this;
        }
        
        Fixed& operator *=(const Fixed& other) {
            // This is 32 lines of assembly when FractionBits<=16 else 55 lines.
            // Using the built-in 64 bit multiplication would result 61 lines.
            
            // Store the sign of result
            std::uint32_t sign = _value ^ other._value;
            
            // Make inputs unsigned
            std::uint32_t u = _value;
            if(u >> 31) u = -u;
            std::uint32_t v = other._value;
            if(v >> 31) v = -v;
            
            // Perform the multiplication
            std::uint32_t result = umul(u, v);
            
            // Restore sign
            _value = (sign >> 31) ? -result : result;
            
            return *this;
        }
        
        Fixed& operator *=(const int& value) {
            _value *= value;
            return *this;
        }
        Fixed& operator *=(const long int& value) {
            _value *= value;
            return *this;
        }
        Fixed& operator *=(const float& value) {
            _value *= Fixed(value);
            return *this;
        }
        Fixed& operator *=(const double& value) {
            _value *= Fixed(value);
            return *this;
        }
        
        Fixed& operator /=(const Fixed& other) {
            PrecisionType num = static_cast<PrecisionType>(_value) << FractionBits;
            _value = num / other._value;
            return *this;
        }
        
        Fixed& operator /=(const int& value) {
            _value /= value;
            return *this;
        }
        Fixed& operator /=(const long int& value) {
            _value /= value;
            return *this;
        }
        Fixed& operator /=(const float& value) {
            _value /= Fixed(value);
            return *this;
        }
        Fixed& operator /=(const double& value) {
            _value /= Fixed(value);
            return *this;
        }
        
        Fixed& operator <<=(const int& shift) {
            _value <<= shift;
            return *this;
        }
        Fixed& operator >>=(const int& shift) {
            _value >>= shift;
            return *this;
        }
    
    private:
    
        class InternalTypeWrapper {
            public:
                constexpr inline explicit InternalTypeWrapper(const InternalType& val) : _value(val) {}
                constexpr inline explicit operator InternalType() const { return _value; }
            
            private:
                const InternalType _value;
        };
        
        // Multiplication of two unsigned 32 bit fixed point numbers
        constexpr std::uint32_t umul(std::uint32_t a, std::uint32_t b) {
            // Split inputs into 16 bit values
            std::uint32_t ah = a >> 16;
            std::uint32_t al = a & 0xffff;
            std::uint32_t bh = b >> 16;
            std::uint32_t bl = b & 0xffff;
            
            // Multiply the values, operation is 32 bit but we need variables to be 64 bit for calculating the sum
            std::uint64_t hh = ah * bh;
            std::uint64_t hl = ah * bl;
            std::uint64_t lh = al * bh;
            std::uint64_t ll = al * bl;
            
            // Sum the parts and shift right by FractionBits
            return (FractionBits > 16) // Compiler picks one of the two based on the template parameter
                ? (hh << (32 - FractionBits)) + ((ll + ((hl + lh) << 16)) >> FractionBits)
                : (hh << (32 - FractionBits)) + (ll >> FractionBits) + ((hl + lh) << (16 - FractionBits));
        }
        
        constexpr inline Fixed(const InternalTypeWrapper& val) : _value(static_cast<InternalType>(val)) {}
        
        InternalType _value;
};


// Comparison Operators

template<unsigned Q>
constexpr bool operator ==(const Fixed<Q>& lhs, const Fixed<Q> rhs) {
    return (lhs.internal() == rhs.internal());
}

template<unsigned Q>
constexpr bool operator !=(const Fixed<Q>& lhs, const Fixed<Q> rhs) {
    return (lhs.internal() != rhs.internal());
}

template<unsigned Q>
constexpr bool operator <(const Fixed<Q>& lhs, const Fixed<Q> rhs) {
    return (lhs.internal() < rhs.internal());
}

template<unsigned Q>
constexpr bool operator >(const Fixed<Q>& lhs, const Fixed<Q> rhs) {
    return (lhs.internal() > rhs.internal());
}

template<unsigned Q>
constexpr bool operator <=(const Fixed<Q>& lhs, const Fixed<Q> rhs) {
    return (lhs.internal() <= rhs.internal());
}

template<unsigned Q>
constexpr bool operator >=(const Fixed<Q>& lhs, const Fixed<Q> rhs) {
    return (lhs.internal() >= rhs.internal());
}


// Arithmetic Operators

template<unsigned Q>
constexpr Fixed<Q> operator +(const Fixed<Q>& lhs, const Fixed<Q>& rhs) {
    return Fixed<Q>::fromInternal(lhs.internal() + rhs.internal());
}

template<unsigned Q>
constexpr Fixed<Q> operator -(const Fixed<Q>& lhs, const Fixed<Q>& rhs) {
    return Fixed<Q>::fromInternal(lhs.internal() - rhs.internal());
}

template<unsigned Q>
constexpr Fixed<Q> operator *(const Fixed<Q>& lhs, const Fixed<Q>& rhs) {
    return Fixed<Q>::fromInternal((static_cast<typename Fixed<Q>::PrecisionType>(lhs.internal()) * static_cast<typename Fixed<Q>::PrecisionType>(rhs.internal())) >> Q);
}

template<unsigned Q>
constexpr Fixed<Q> operator *(const Fixed<Q>& lhs, const int& rhs) {
    return Fixed<Q>::fromInternal(lhs.internal() * rhs);
}

template<unsigned Q>
constexpr Fixed<Q> operator *(const int& lhs, const Fixed<Q>& rhs) {
    return Fixed<Q>::fromInternal(lhs * rhs.internal());
}

template<unsigned Q>
constexpr Fixed<Q> operator *(const Fixed<Q>& lhs, const long int& rhs) {
    return Fixed<Q>::fromInternal(lhs.internal() * rhs);
}

template<unsigned Q>
constexpr Fixed<Q> operator *(const long int& lhs, const Fixed<Q>& rhs) {
    return Fixed<Q>::fromInternal(lhs * rhs.internal());
}

template<unsigned Q>
constexpr Fixed<Q> operator *(const Fixed<Q>& lhs, const float& rhs) {
    return lhs * Fixed<Q>(rhs);
}

template<unsigned Q>
constexpr Fixed<Q> operator *(const float& lhs, const Fixed<Q>& rhs) {
    return Fixed<Q>(lhs) * rhs;
}

template<unsigned Q>
constexpr Fixed<Q> operator *(const Fixed<Q>& lhs, const double& rhs) {
    return lhs* Fixed<Q>(rhs);
}

template<unsigned Q>
constexpr Fixed<Q> operator *(const double& lhs, const Fixed<Q>& rhs) {
    return Fixed<Q>(lhs) * rhs;
}

template<unsigned Q>
constexpr Fixed<Q> operator /(const Fixed<Q>& lhs, const Fixed<Q>& rhs) {
    return Fixed<Q>::fromInternal((static_cast<typename Fixed<Q>::PrecisionType>(lhs.internal()) << Q) / rhs.internal());
}

template<unsigned Q>
constexpr Fixed<Q> operator /(const Fixed<Q>& lhs, const int& rhs) {
    return Fixed<Q>::fromInternal(lhs.internal() / rhs);
}

template<unsigned Q>
constexpr Fixed<Q> operator /(const Fixed<Q>& lhs, const long int& rhs) {
    return Fixed<Q>::fromInternal(lhs.internal() / rhs);
}

template<unsigned Q>
constexpr Fixed<Q> operator /(const Fixed<Q>& lhs, const float& rhs) {
    return lhs / Fixed<Q>(rhs);
}

template<unsigned Q>
constexpr Fixed<Q> operator /(const Fixed<Q>& lhs, const double& rhs) {
    return lhs / Fixed<Q>(rhs);
}

template<unsigned Q>
constexpr Fixed<Q> operator <<(const Fixed<Q>& lhs, const int& rhs) {
    return Fixed<Q>::fromInternal(lhs.internal() << rhs);
}

template<unsigned Q>
constexpr Fixed<Q> operator >>(const Fixed<Q>& lhs, const int& rhs) {
    return Fixed<Q>::fromInternal(lhs.internal() >> rhs);
}

} // namespace FixedPoint
