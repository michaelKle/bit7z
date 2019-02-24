// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip DLLs.
 * Copyright (c) 2014-2018  Riccardo Ostani - All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * Bit7z is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with bit7z; if not, see https://www.gnu.org/licenses/.
 */

#include "../include/bitmemextractor.hpp"

#include "7zip/Archive/IArchive.h"

#include "../include/bitexception.hpp"
#include "../include/opencallback.hpp"
#include "../include/memextractcallback.hpp"
#include "../include/extractcallback.hpp"
#include "../include/util.hpp"

using namespace bit7z;
using namespace bit7z::util;
using namespace std;
using namespace NWindows;
using namespace NArchive;

BitMemExtractor::BitMemExtractor( const Bit7zLibrary& lib, const BitInFormat& format )
    : BitArchiveOpener( lib, format ) {
    if ( format == BitFormat::Auto ) {
        throw BitException( "Automatic format detection not supported for in-memory archives" );
    }
}

void BitMemExtractor::extract( const vector< byte_t >& in_buffer, const wstring& out_dir ) const {
    CMyComPtr< IInArchive > in_archive = openArchive( *this, mFormat, in_buffer );

    auto* extract_callback_spec = new ExtractCallback( *this, in_archive, L"", out_dir );

    CMyComPtr< IArchiveExtractCallback > extract_callback( extract_callback_spec );
    if ( in_archive->Extract( nullptr, static_cast< uint32_t >( -1 ), NExtract::NAskMode::kExtract, extract_callback ) != S_OK ) {
        throw BitException( extract_callback_spec->getErrorMessage() );
    }
}

void BitMemExtractor::extract( const vector< byte_t >& in_buffer, vector< byte_t >& out_buffer,
                               unsigned int index ) const {
    CMyComPtr< IInArchive > in_archive = openArchive( *this, mFormat, in_buffer );

    uint32_t number_items;
    in_archive->GetNumberOfItems( &number_items );
    if ( index >= number_items ) {
        throw BitException( "Index " + std::to_string( index ) + " is out of range"  );
    }

    auto* extract_callback_spec = new MemExtractCallback( *this, in_archive, out_buffer );

    const uint32_t indices[] = { index };

    CMyComPtr< IArchiveExtractCallback > extract_callback( extract_callback_spec );
    if ( in_archive->Extract( indices, 1, NExtract::NAskMode::kExtract, extract_callback ) != S_OK ) {
        throw BitException( extract_callback_spec->getErrorMessage() );
    }
}
