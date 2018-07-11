/**
 *
 * peer_connector_interface
 *
 * peer define
 *
 * @author  :   yandaren1220@126.com
 * @date    :   2017-03-30
 */

#ifndef __common_rudp_netpeer_define_h__
#define __common_rudp_netpeer_define_h__

#include "peer_config.h"
#include <stdint.h>

namespace netpeer
{
    /// These enumerations are used to describe when packets are delivered.
    enum packet_priority
    {
        /// The highest possible priority. These message trigger sends immediately, and are generally not buffered or aggregated into a single datagram.
        priority_immediate,

        /// For every 2 IMMEDIATE_PRIORITY messages, 1 HIGH_PRIORITY will be sent.
        /// Messages at this priority and lower are buffered to be sent in groups at 10 millisecond intervals to reduce UDP overhead and better measure congestion control. 
        priority_high,

        /// For every 2 HIGH_PRIORITY messages, 1 MEDIUM_PRIORITY will be sent.
        /// Messages at this priority and lower are buffered to be sent in groups at 10 millisecond intervals to reduce UDP overhead and better measure congestion control. 
        priority_medium,

        /// For every 2 MEDIUM_PRIORITY messages, 1 LOW_PRIORITY will be sent.
        /// Messages at this priority and lower are buffered to be sent in groups at 10 millisecond intervals to reduce UDP overhead and better measure congestion control. 
        priority_low,

        /// \internal
        priority_num,
    };

    /// These enumerations are used to describe how packets are delivered.
    /// \note  Note to self: I write this with 3 bits in the stream.  If I add more remember to change that
    /// \note In ReliabilityLayer::WriteToBitStreamFromInternalPacket I assume there are 5 major types
    /// \note Do not reorder, I check on >= UNRELIABLE_WITH_ACK_RECEIPT
    enum packet_reliability
    {
        /// Same as regular UDP, except that it will also discard duplicate datagrams.  RakNet adds (6 to 17) + 21 bits of overhead, 16 of which is used to detect duplicate packets and 6 to 17 of which is used for message length.
        unreliable,

        /// Regular UDP with a sequence counter.  Out of order messages will be discarded.
        /// Sequenced and ordered messages sent on the same channel will arrive in the order sent.
        unreliable_sequenced,

        /// The message is sent reliably, but not necessarily in any order.  Same overhead as UNRELIABLE.
        reliable,

        /// This message is reliable and will arrive in the order you sent it.  Messages will be delayed while waiting for out of order messages.  Same overhead as UNRELIABLE_SEQUENCED.
        /// Sequenced and ordered messages sent on the same channel will arrive in the order sent.
        reliable_ordered,

        /// This message is reliable and will arrive in the sequence you sent it.  Out or order messages will be dropped.  Same overhead as UNRELIABLE_SEQUENCED.
        /// Sequenced and ordered messages sent on the same channel will arrive in the order sent.
        reliable_sequenced,

        /// Same as UNRELIABLE, however the user will get either ID_SND_RECEIPT_ACKED or ID_SND_RECEIPT_LOSS based on the result of sending this message when calling RakPeerInterface::Receive(). Bytes 1-4 will contain the number returned from the Send() function. On disconnect or shutdown, all messages not previously acked should be considered lost.
        unreliable_with_ack_receipt,

        /// Same as UNRELIABLE_SEQUENCED, however the user will get either ID_SND_RECEIPT_ACKED or ID_SND_RECEIPT_LOSS based on the result of sending this message when calling RakPeerInterface::Receive(). Bytes 1-4 will contain the number returned from the Send() function. On disconnect or shutdown, all messages not previously acked should be considered lost.
        /// 05/04/10 You can't have sequenced and ack receipts, because you don't know if the other system discarded the message, meaning you don't know if the message was processed
        // UNRELIABLE_SEQUENCED_WITH_ACK_RECEIPT,

        /// Same as RELIABLE. The user will also get ID_SND_RECEIPT_ACKED after the message is delivered when calling RakPeerInterface::Receive(). ID_SND_RECEIPT_ACKED is returned when the message arrives, not necessarily the order when it was sent. Bytes 1-4 will contain the number returned from the Send() function. On disconnect or shutdown, all messages not previously acked should be considered lost. This does not return ID_SND_RECEIPT_LOSS.
        reliable_with_ack_receipt,

        /// Same as RELIABLE_ORDERED_ACK_RECEIPT. The user will also get ID_SND_RECEIPT_ACKED after the message is delivered when calling RakPeerInterface::Receive(). ID_SND_RECEIPT_ACKED is returned when the message arrives, not necessarily the order when it was sent. Bytes 1-4 will contain the number returned from the Send() function. On disconnect or shutdown, all messages not previously acked should be considered lost. This does not return ID_SND_RECEIPT_LOSS.
        reliable_ordered_with_ack_receipt,

        /// Same as RELIABLE_SEQUENCED. The user will also get ID_SND_RECEIPT_ACKED after the message is delivered when calling RakPeerInterface::Receive(). Bytes 1-4 will contain the number returned from the Send() function. On disconnect or shutdown, all messages not previously acked should be considered lost.
        /// 05/04/10 You can't have sequenced and ack receipts, because you don't know if the other system discarded the message, meaning you don't know if the message was processed
        // RELIABLE_SEQUENCED_WITH_ACK_RECEIPT,

        /// \internal
        reliabilitys_num,
    };

    // log level
    enum peer_log_level
    {
        peer_log_level_debug = 0,
        peer_log_level_info = 1,
        peer_log_level_warn = 2,
        peer_log_level_error = 3,
        peer_log_level_fatal = 4,
    };


    // peer net statistics
    enum nps_per_second_metrisc
    {
        /// How many bytes per pushed via a call to RakPeerInterface::Send()
        user_message_bytes_pushed = 0,

        /// How many user message bytes were sent via a call to RakPeerInterface::Send(). This is less than or equal to USER_MESSAGE_BYTES_PUSHED.
        /// A message would be pushed, but not yet sent, due to congestion control
        user_message_bytes_sent,

        /// How many user message bytes were resent. A message is resent if it is marked as reliable, and either the message didn't arrive or the message ack didn't arrive.
        user_message_bytes_resent,

        /// How many user message bytes were received, and returned to the user successfully.
        user_message_bytes_received_processed,

        /// How many user message bytes were received, but ignored due to data format errors. This will usually be 0.
        user_message_bytes_received_ignored,

        /// How many actual bytes were sent, including per-message and per-datagram overhead, and reliable message acks
        actual_bytes_sent,

        /// How many actual bytes were received, including overead and acks.
        actual_bytes_received,

        /// \internal
        nps_per_second_metrics_count,
    };

    struct np_api np_statistics
    {
        // for each type in nps_per_second_metrisc, what is the value over the last 1 second?
        uint64_t last_second[nps_per_second_metrics_count];

        // for each type in nps_per_second_metrisc, what is the total value over the lifetime of the connection?
        uint64_t running_total[nps_per_second_metrics_count];

        // when did the connection start?, in unix timstamp
        uint64_t connection_start_time;
    };

}

#endif