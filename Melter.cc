/*
 * Melter.cpp
 *
 *  Created on: Oct 4, 2010
 *      Author: daniele
 */

#include <string>
#include <iostream>

#include "Melter.h"
#include "Chunk.h"
#include "IOManager.h"
#include "Mangler.h"

Melter::Melter(::prototype::Parser& parser, ::prototype::Mangler& mangler)
: _parser(parser), _mangler(mangler), _defective(false)
{
    _io = new IOManager();
    _io->register_source_observer(*this);
}

Melter::~Melter()
{
    delete _io;
}

Chunk Melter::output()
{
    return _io->read_from_output();
}

void Melter::update()
{
    if (!_io)
        assert(false);
    
    while ( _io->available( _parser.descriptor()  ) ) {
        std::string tag = _parser.tag();
        
        // get chunk and parse it, check for action expiration
        Chunk chunk = _io->read_for_parsing( _parser.descriptor() );
        bool expired = _parser.call( chunk );
        
        _mangler.call(*_io, _parser.tag(), chunk);
        
        if (expired)
            _parser.expire_action();
    }
    
    _io->process_pending();
}

std::size_t Melter::write( char const * data, std::size_t size )
{
    if (!data)
        return 0;
    
    Chunk chunk = Chunk( data, BufferDescriptor(0, size) );
    return this->write( chunk );
}

std::size_t Melter::write( Chunk &chunk )
{
    return _io->append_to_input( chunk );
}

Chunk Melter::read()
{
    Chunk chunk = _io->read_from_output();
    return chunk;
}