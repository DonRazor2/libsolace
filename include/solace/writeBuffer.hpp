/*
*  Copyright 2018 Ivan Ryabov
*
*  Licensed under the Apache License, Version 2.0 (the "License");
*  you may not use this file except in compliance with the License.
*  You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
*  Unless required by applicable law or agreed to in writing, software
*  distributed under the License is distributed on an "AS IS" BASIS,
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*  See the License for the specific language governing permissions and
*  limitations under the License.
*/
/*******************************************************************************
 * libSolace: Write buffer
 *	@file		solace/WriteBuffer.hpp
 *	@brief		Write-only adapter for a buffer.
 ******************************************************************************/
#pragma once
#ifndef SOLACE_WRITEBUFFER_HPP
#define SOLACE_WRITEBUFFER_HPP

#include "solace/memoryView.hpp"
#include "solace/memoryBuffer.hpp"

#include "solace/result.hpp"
#include "solace/error.hpp"


namespace Solace {


/**
 * Write-only adapter for a memory buffer.
 */
class WriteBuffer {
public:

    using Storage = MemoryBuffer;
    using size_type = Storage::size_type;

public:

    /** Construct an empty buffer of size zero */
    WriteBuffer() noexcept = default;

    WriteBuffer(WriteBuffer const& other) = delete;
    WriteBuffer& operator= (WriteBuffer const&) = delete;


    /**
     * Construct the byte buffer by moving content from the other buffer
     * @param other Other buffer to take over from
     */
    WriteBuffer(WriteBuffer&& other) = default;

    WriteBuffer(MemoryBuffer& buffer) :
        _limit(buffer.size()),
        _storage(buffer.view(), nullptr)
    {}

    WriteBuffer(MemoryBuffer&& buffer) :
        _limit(buffer.size()),
        _storage(std::move(buffer))
    {}

    /**
     * Construct the byte buffer from the memory view object
     * @param other Other buffer to copy data from
     */
    WriteBuffer(MemoryView memView) :
        _limit(memView.size()),
        _storage(std::move(memView), nullptr)
    {}


    WriteBuffer& swap(WriteBuffer& rhs) noexcept {
        using std::swap;

        swap(_position, rhs._position);
        swap(_limit, rhs._limit);
        swap(_storage, rhs._storage);

        return *this;
    }

    WriteBuffer& operator= (WriteBuffer&& rhs) noexcept {
        return swap(rhs);
    }


    /**
     * Leave the limit unchanged and sets the position to zero.
     * It alwats works.
     * @return A reference to this for fluency.
     */
    WriteBuffer& rewind() noexcept {
        _position = 0;

        return *this;
    }


    /** Returns this buffer's capacity.
     * A buffer's capacity is the number of elements it contains.
     * The capacity of a buffer is never negative and never changes.
     * @return Capacity of the buffer in bytes.
     */
    size_type capacity() const noexcept { return _storage.size(); }

    /** Return data read/write limit of this buffer.
     * For write buffer this is the maximum number of bytes this buffer can hold.
     * For read buffer this is number of bytes that can be read from this buffer.
     * @note Buffer limit is always less of equal to buffer's capacity.
     * @return Number of bytes that can be read/written to the buffer
     */
    size_type limit() const noexcept { return _limit; }

    /** Limit number of bytes that can be read/written to this buffer.

     * @note Buffer limit is always less of equal to buffer's capacity.
     * @return A reference to this this buffer.
     *
     * @throws IllegalArgumentException if attempt is made to set limit to more then this buffer capacity.
     */
    Result<void, Error> limit(size_type newLimit);

    /**
     * Get remaining number of bytes in the buffer (Up to the limit)
     * @return Remaining number of bytes in the buffer.
     */
    size_type remaining() const noexcept { return limit() - position(); }

    /**
     * Check if there are bytes left in the buffer (Up to the limit)
     * @return True if there are still some data before the limit is reached.
     */
    bool hasRemaining() const noexcept { return remaining() > 0; }

    /**
     * Set position back to the previously saved mark
     * @return Reference to this buffer.
     */
    Result<void, Error>
    reset(size_type savedMark) {
        return position(savedMark);
    }

    /**
     * Get current position in the buffer.
     * It can be stored to later return to it using @see reset
     * @return Current position in the buffer
     */
    size_type position() const noexcept { return _position; }

    /**
     * Set current position to the given one.
     *
     * @return Reference to this buffer for fluent interface.
     * @note It is illigal to set position beyond the limit(), and exception will be raised in that case.
     */
    Result<void, Error> position(size_type newPosition);

    /**
     * Increment current position by the given amount.
     * @param increment Amount to advance current position by.
     * @return Reference to this buffer for fluent interface.
     * @note It is illigal to advance position beyond the limit(), and exception will be raised in that case.
     */
    Result<void, Error> advance(size_type increment);

    /**
     * Set the limit to the capacity and the position to zero.
     */
    WriteBuffer& clear() noexcept {
        _position = 0;
        _limit = capacity();

        return *this;
    }

    /**
     * Set the limit to the current position and then sets the position to zero.
     */
    WriteBuffer& flip() noexcept {
        _limit = _position;
        _position = 0;

        return *this;
    }


    ImmutableMemoryView viewRemaining() const {
        return _storage.view().slice(position(), limit());
    }

    MemoryView viewRemaining() {
        return _storage.view().slice(position(), limit());
    }

    ImmutableMemoryView viewWritten() const {
        return _storage.view().slice(0, position());
    }

    MemoryView viewWritten() {
        return _storage.view().slice(0, position());
    }


    /**
     * Write given raw bytes into this buffer.
     * @param data Raw bytes data to write.
     * @return Result of write operation.
     */
    Result<void, Error> write(ImmutableMemoryView const& data) {
        return write(data.dataAddress(), data.size());
    }

    /**
     * Write given raw bytes into this buffer.
     * @param data Raw bytes data to write.
     * @param bytesToWrite Number of bytes to write from data into this buffer.
     * @return Refernce to this for luency.
     * @note Exception is thrown if bytesToWrite exceed buffer capacity.
     */
    Result<void, Error> write(ImmutableMemoryView const& data, size_type bytesToWrite);

    Result<void, Error> write(char value)       { return write(&value, sizeof(value)); }
    Result<void, Error> write(int8 value)       { return write(&value, sizeof(value)); }
    Result<void, Error> write(uint8 value)      { return write(&value, sizeof(value)); }
    Result<void, Error> write(int16 value)      { return write(&value, sizeof(value)); }
    Result<void, Error> write(uint16 value)     { return write(&value, sizeof(value)); }
    Result<void, Error> write(int32 value)      { return write(&value, sizeof(value)); }
    Result<void, Error> write(uint32 value)     { return write(&value, sizeof(value)); }
    Result<void, Error> write(int64 value)      { return write(&value, sizeof(value)); }
    Result<void, Error> write(uint64 value)     { return write(&value, sizeof(value)); }
    Result<void, Error> write(float32 value)    { return write(&value, sizeof(value)); }
    Result<void, Error> write(float64 value)    { return write(&value, sizeof(value)); }

    // Endianess aware write methods
    Result<void, Error> writeLE(int8 value)  { return write(&value, sizeof(value)); }
    Result<void, Error> writeLE(uint8 value) { return write(&value, sizeof(value)); }
    Result<void, Error> writeLE(int16 value) { return writeLE(static_cast<uint16>(value)); }
    Result<void, Error> writeLE(uint16 value);
    Result<void, Error> writeLE(int32 value) { return writeLE(static_cast<uint32>(value)); }
    Result<void, Error> writeLE(uint32 value);
    Result<void, Error> writeLE(int64 value) { return writeLE(static_cast<uint64>(value)); }
    Result<void, Error> writeLE(uint64 value);

    Result<void, Error> writeBE(int8 value)  { return write(&value, sizeof(value)); }
    Result<void, Error> writeBE(uint8 value) { return write(&value, sizeof(value)); }
    Result<void, Error> writeBE(int16 value) { return writeBE(static_cast<uint16>(value)); }
    Result<void, Error> writeBE(uint16 value);
    Result<void, Error> writeBE(int32 value) { return writeBE(static_cast<uint32>(value)); }
    Result<void, Error> writeBE(uint32 value);
    Result<void, Error> writeBE(int64 value) { return writeBE(static_cast<uint64>(value)); }
    Result<void, Error> writeBE(uint64 value);

protected:

    Result<void, Error> write(void const* bytes, size_type count);

private:

    size_type           _position{};
    size_type           _limit{};

    Storage             _storage;
};


inline void swap(WriteBuffer& lhs, WriteBuffer& rhs) noexcept {
    lhs.swap(rhs);
}

}  // End of namespace Solace
#endif  // SOLACE_WRITEBUFFER_HPP
