/*
*  Copyright 2016 Ivan Ryabov
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
 * libSolace: Async event
 *	@file		solace/io/async/pipe.hpp
 *	@author		$LastChangedBy$
 *	@date		$LastChangedDate$
 *	ID:			$Id$
 ******************************************************************************/
#pragma once
#ifndef SOLACE_IO_ASYNC_UNIXSOCKET_HPP
#define SOLACE_IO_ASYNC_UNIXSOCKET_HPP

#include "solace/io/selectable.hpp"
#include "solace/io/async/future.hpp"
#include "solace/io/async/channel.hpp"

#include "solace/string.hpp"


namespace Solace { namespace IO { namespace async {

class UnixSocket: public Channel {
public:
    using Channel::size_type;

    typedef String endpoint_type;

public:

    ~UnixSocket();

    UnixSocket(const UnixSocket& rhs) = delete;
    UnixSocket& operator= (const UnixSocket& rhs) = delete;

    UnixSocket(EventLoop& ioContext);

    UnixSocket(UnixSocket&& rhs);

    UnixSocket& operator= (UnixSocket&& rhs) noexcept {
        return swap(rhs);
    }

    UnixSocket& swap(UnixSocket& rhs) noexcept {
        using std::swap;

        Channel::swap(rhs);
        swap(_fd, rhs._fd);

        return *this;
    }


    /**
     * Start an syncronous connection to the given endpoint.
     * This call will block until a connection is complete (either successfully or in an error)
     * @param endpoint An endpoint to connect to.
     */
    void connect(const endpoint_type& endpoint);

    /**
     * Start an asynchronous connection to the given endpoint.
     * @param endpoint An endpoint to connect to.
     * @return Future that is resolved when connection is establised or an error occured.
     */
    Future<void> asyncConnect(const endpoint_type& endpoint);

    /**
     * Post an async read request to read data from this IO object into the given buffer.
     * This method reads the data until the provided destination buffer is full.
     *
     * @param dest The provided destination buffer to read data into.
     * @return A future that will be resolved one the buffer has been filled.
     */
    Future<void> asyncRead(ByteBuffer& dest) {
        return asyncRead(dest, dest.remaining());
    }

    /**
     * Post an async read request to read specified amount of data from this IO object into the given buffer.
     *
     * @param dest The provided destination buffer to read data into.
     * @param bytesToRead Amount of data (in bytes) to read from this IO object.
     * @return A future that will be resolved one the scpecified number of bytes has been read.
     *
     * @note If the provided destination buffer is too small to hold requested amount of data - an exception is raised.
     */
    Future<void> asyncRead(ByteBuffer& dest, size_type bytesToRead);

    /**
     * Post an async write request to write specified amount of data into this IO object.
     * This method writes whole content of the provided buffer into the IO objec.
     *
     * @param src The provided source buffer to read data from.
     * @return A future that will be resolved one the scpecified number of bytes has been written into the IO object.
     */
    Future<void> asyncWrite(ByteBuffer& src) {
        return asyncWrite(src, src.remaining());
    }

    /**
     * Post an async write request to write specified amount of data into this IO object.
     *
     * @param src The provided source buffer to read data from.
     * @param bytesToRead Amount of data (in bytes) to write from the buffer into this IO object.
     * @return A future that will be resolved one the scpecified number of bytes has been written into the IO object.
     *
     * @note If the provided source buffer does not have requested amount of data - an exception is raised.
     */
    Future<void> asyncWrite(ByteBuffer& src, size_type bytesToRead);

private:

    ISelectable::poll_id _fd;
};


inline void swap(UnixSocket& lhs, UnixSocket& rhs) noexcept {
    lhs.swap(rhs);
}

}  // End of namespace async
}  // End of namespace IO
}  // End of namespace Solace
#endif  // SOLACE_IO_ASYNC_UNIXSOCKET_HPP
