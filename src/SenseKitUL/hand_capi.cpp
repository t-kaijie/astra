#include <SenseKit/sensekit_types.h>
#include "generic_stream_api.h"
#include <math.h>
#include <memory.h>
#include <SenseKitUL/StreamTypes.h>
#include <SenseKitUL/Plugins/stream_types.h>
#include <SenseKitUL/streams/hand_capi.h>
#include <string.h>
#include <SenseKitUL/streams/image_capi.h>

SENSEKIT_BEGIN_DECLS

SENSEKIT_API_EX sensekit_status_t sensekit_reader_get_handstream(sensekit_reader_t reader,
                                                           sensekit_handstream_t* handStream)

{
    return sensekit_reader_get_stream(reader,
                                      SENSEKIT_STREAM_HAND,
                                      DEFAULT_SUBTYPE,
                                      handStream);
}

SENSEKIT_API_EX sensekit_status_t sensekit_frame_get_handframe(sensekit_reader_frame_t readerFrame,
                                                          sensekit_handframe_t* handFrame)
{
    return sensekit_generic_frame_get<sensekit_handframe_wrapper_t>(readerFrame,
                                                                    SENSEKIT_STREAM_HAND,
                                                                    DEFAULT_SUBTYPE,
                                                                    handFrame);
}

SENSEKIT_API_EX sensekit_status_t sensekit_frame_get_handframe_with_subtype(sensekit_reader_frame_t readerFrame,
                                                                            sensekit_stream_subtype_t subtype, 
                                                                            sensekit_handframe_t* handFrame)
{
    return sensekit_generic_frame_get<sensekit_handframe_wrapper_t>(readerFrame,
                                                                    SENSEKIT_STREAM_HAND,
                                                                    subtype,
                                                                    handFrame);
}

SENSEKIT_API_EX sensekit_status_t sensekit_handframe_get_frameindex(sensekit_handframe_t handFrame,
                                                                     sensekit_frame_index_t* index)
{
    return sensekit_generic_frame_get_frameindex(handFrame, index);
}

SENSEKIT_API_EX sensekit_status_t sensekit_handframe_get_hand_count(sensekit_handframe_t handFrame,
                                                                   size_t* numHands)
{
    *numHands = handFrame->numHands;

    return SENSEKIT_STATUS_SUCCESS;
}

SENSEKIT_API_EX sensekit_status_t sensekit_handframe_copy_hands(sensekit_handframe_t handFrame,
                                                                sensekit_handpoint_t* handPointsDestination)
{
    size_t size = handFrame->numHands * sizeof(sensekit_handpoint_t);

    memcpy(handPointsDestination, handFrame->handpoints, size);

    return SENSEKIT_STATUS_SUCCESS;
}

SENSEKIT_API_EX sensekit_status_t sensekit_handframe_get_hands_ptr(sensekit_handframe_t handFrame,
                                                                   sensekit_handpoint_t** handpoints,
                                                                   size_t* numHands)
{
    *handpoints = handFrame->handpoints;
    sensekit_handframe_get_hand_count(handFrame, numHands);

    return SENSEKIT_STATUS_SUCCESS;
}

SENSEKIT_API_EX sensekit_status_t sensekit_reader_get_debug_handstream(sensekit_reader_t reader,
                                                                       sensekit_debug_handstream_t* debugHandStream)

{
    return sensekit_reader_get_stream(reader,
                                      SENSEKIT_STREAM_DEBUG_HAND,
                                      DEFAULT_SUBTYPE,
                                      debugHandStream);
}

SENSEKIT_API_EX sensekit_status_t sensekit_frame_get_debug_handframe(sensekit_reader_frame_t readerFrame,
                                                                     sensekit_debug_handframe_t* debugHandFrame)
{
    return sensekit_reader_get_imageframe(readerFrame,
                                          SENSEKIT_STREAM_DEBUG_HAND,
                                          DEFAULT_SUBTYPE,
                                          debugHandFrame);
}


SENSEKIT_END_DECLS
