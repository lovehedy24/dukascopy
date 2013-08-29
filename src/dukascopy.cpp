/**
 *
 */


#include "ninety47/dukascopy.h"
#include "ninety47/dukascopy/defs.h"
#include "ninety47/dukascopy/lzma.h"

#include <boost/date_time/posix_time/posix_time.hpp>
#include <cstdlib>
#include <algorithm>
#include <vector>


namespace n47 {

namespace pt = boost::posix_time;

tick *tickFromBuffer(unsigned char *buffer, pt::ptime epoch, float digits, size_t offset){
    bytesTo<unsigned int, n47::BigEndian> bytesTo_unsigned;
    bytesTo<float, n47::BigEndian> bytesTo_float;

    unsigned int ts = bytesTo_unsigned( buffer + offset );
    pt::time_duration ms = pt::millisec(ts);
    unsigned int ofs = offset + sizeof(unsigned int);
    float ask = bytesTo_unsigned(buffer + ofs) * digits;
    ofs += sizeof(int);
    float bid = bytesTo_unsigned(buffer + ofs) * digits;
    ofs += sizeof(int);
    float askv = bytesTo_float(buffer + ofs);
    ofs += sizeof(int);
    float bidv = bytesTo_float(buffer + ofs);

    return new tick(epoch, ms, ask, bid, askv, bidv);
}


tick_data* read_bin(unsigned char *buffer, size_t buffer_size, pt::ptime epoch, float point_value) {
    std::vector<tick*> *data = new std::vector<tick*>();
    std::vector<tick*>::iterator iter;

    std::size_t offset = 0;

    while ( offset < buffer_size ) {
        data->push_back( tickFromBuffer(buffer, epoch, point_value, offset) );
        offset += ROW_SIZE;
    }

    return data;
}


tick_data* read_bi5(unsigned char *lzma_buffer, size_t lzma_buffer_size, pt::ptime epoch, float point_value, size_t *buffer_size) {
    //unsigned char *buffer = 0;
    size_t outSize = 0;

    // decompress
    int status;
    unsigned char *buffer = n47::lzma::decompress(lzma_buffer, lzma_buffer_size, &status, &outSize);

    if (status != N47_E_OK) {
        *buffer_size = 0;
        return 0;
    } else {
        *buffer_size = outSize;
        // convert to tick data (with read_bin).
        return read_bin(buffer, outSize, epoch, point_value);
    }
}


} // namespace n47

