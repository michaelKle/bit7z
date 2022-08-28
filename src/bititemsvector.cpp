// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "bititemsvector.hpp"

#include "bitexception.hpp"
#include "internal/bufferitem.hpp"
#include "internal/fsindexer.hpp"
#include "internal/stdinputitem.hpp"

using namespace bit7z;
using filesystem::FSItem;
using filesystem::FSIndexer;

void BitItemsVector::indexDirectory( const fs::path& in_dir, const tstring& filter, bool recursive ) {
    FSItem dir_item{ in_dir }; //Note: if in_dir is an invalid path, FSItem constructor throws a BitException!
    if ( filter.empty() && !dir_item.inArchivePath().empty() ) {
        mItems.emplace_back( std::make_unique< FSItem >( dir_item ) );
    }
    FSIndexer indexer{ dir_item, filter };
    indexer.listDirectoryItems( mItems, recursive );
}

void BitItemsVector::indexPaths( const vector< tstring >& in_paths, bool ignore_dirs ) {
    for ( const auto& file_path : in_paths ) {
        FSItem item{ file_path };
        indexItem( item, ignore_dirs );
    }
}

void BitItemsVector::indexPathsMap( const map< tstring, tstring >& in_paths, bool ignore_dirs ) {
    for ( const auto& file_pair : in_paths ) {
        FSItem item{ fs::path( file_pair.first ), fs::path( file_pair.second ) };
        indexItem( item, ignore_dirs );
    }
}

void BitItemsVector::indexItem( const FSItem& item, bool ignore_dirs ) {
    if ( !item.isDir() ) {
        mItems.emplace_back( std::make_unique< FSItem >( item ) );
    } else if ( !ignore_dirs ) { //item is a directory
        if ( !item.inArchivePath().empty() ) {
            mItems.emplace_back( std::make_unique< FSItem >( item ) );
        }
        FSIndexer indexer{ item };
        indexer.listDirectoryItems( mItems, true );
    } else {
        // No action needed
    }
}

void BitItemsVector::indexFile( const tstring& in_file, const tstring& name ) {
    if ( fs::is_directory( in_file ) ) {
        throw BitException( "Input path points to a directory, not a file",
                            std::make_error_code( std::errc::invalid_argument ), in_file );
    }
    mItems.emplace_back( std::make_unique< FSItem >( in_file, name ) );
}

void BitItemsVector::indexBuffer( const vector< byte_t >& in_buffer, const tstring& name ) {
    mItems.emplace_back( std::make_unique< BufferItem >( in_buffer, name ) );
}

void BitItemsVector::indexStream( std::istream& in_stream, const tstring& name ) {
    mItems.emplace_back( std::make_unique< StdInputItem >( in_stream, name ) );
}

size_t BitItemsVector::size() const {
    return mItems.size();
}

const GenericInputItem& BitItemsVector::operator[]( GenericInputItemVector::size_type index ) const {
    // Note: here index is expected to be correct!
    return *mItems[ index ];
}

GenericInputItemVector::const_iterator BitItemsVector::begin() const noexcept {
    return mItems.cbegin();
}

GenericInputItemVector::const_iterator BitItemsVector::end() const noexcept {
    return mItems.cend();
}

GenericInputItemVector::const_iterator BitItemsVector::cbegin() const noexcept {
    return mItems.cbegin();
}

GenericInputItemVector::const_iterator BitItemsVector::cend() const noexcept {
    return mItems.cend();
}

/* Note: separate declaration/definition of the default destructor is needed to use an incomplete type
 *       for the unique_ptr objects stored in the vector. */
BitItemsVector::~BitItemsVector() = default;