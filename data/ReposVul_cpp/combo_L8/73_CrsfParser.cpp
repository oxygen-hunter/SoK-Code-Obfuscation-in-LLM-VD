import ctypes
from ctypes import c_uint8, c_uint32, c_bool, POINTER

lib = ctypes.CDLL('./crsf_parser_lib.so')

CRSF_CHANNEL_VALUE_MIN = 172
CRSF_CHANNEL_VALUE_MAX = 1811
CRSF_CHANNEL_VALUE_SPAN = CRSF_CHANNEL_VALUE_MAX - CRSF_CHANNEL_VALUE_MIN
CRSF_MAX_PACKET_LEN = 64
CRSF_HEADER = 0xc8

CRSF_CHANNEL_COUNT = 16
RX_QUEUE_BUFFER_SIZE = 200

parser_state = 0
working_index = 0
working_segment_size = 1

rx_queue_buffer = (c_uint8 * RX_QUEUE_BUFFER_SIZE)()
process_buffer = (c_uint8 * CRSF_MAX_PACKET_LEN)()
working_descriptor = None

# Define CRSF_PACKET_DESCRIPTOR_COUNT, crsf_packet_descriptors, etc. as per the original C/C++ code

class CrsfPacket_t(ctypes.Structure):
    pass

class CrsfParserStatistics_t(ctypes.Structure):
    pass

def CrsfParser_Init():
    lib.QueueBuffer_Init(ctypes.byref(rx_queue), rx_queue_buffer, RX_QUEUE_BUFFER_SIZE)

def ConstrainF(x, min_val, max_val):
    return min_val if x < min_val else max_val if x > max_val else x

def MapF(x, in_min, in_max, out_min, out_max):
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min

def ProcessChannelData(data, size, new_packet):
    raw_channels = (c_uint32 * CRSF_CHANNEL_COUNT)()
    new_packet.contents.message_type = 1

    raw_channels[0] = (data[0] | data[1] << 8) & 0x07FF
    # Continue decoding as per the original C/C++ code

    for i in range(CRSF_CHANNEL_COUNT):
        raw_channels[i] = ConstrainF(raw_channels[i], CRSF_CHANNEL_VALUE_MIN, CRSF_CHANNEL_VALUE_MAX)
        new_packet.contents.channel_data.channels[i] = MapF(float(raw_channels[i]), CRSF_CHANNEL_VALUE_MIN, CRSF_CHANNEL_VALUE_MAX, 1000.0, 2000.0)

    return True

def ProcessLinkStatistics(data, size, new_packet):
    new_packet.contents.message_type = 2

    new_packet.contents.link_statistics.uplink_rssi_1 = data[0]
    # Continue processing as per the original C/C++ code

    return True

def FindCrsfDescriptor(packet_type):
    for descriptor in crsf_packet_descriptors:
        if descriptor.packet_type == packet_type:
            return descriptor
    return None

def CrsfParser_LoadBuffer(buffer, size):
    return lib.QueueBuffer_AppendBuffer(ctypes.byref(rx_queue), buffer, size)

def CrsfParser_FreeQueueSize():
    return RX_QUEUE_BUFFER_SIZE - lib.QueueBuffer_Count(ctypes.byref(rx_queue))

def CrsfParser_TryParseCrsfPacket(new_packet, parser_statistics):
    global working_index, working_segment_size, parser_state
    buffer_count = lib.QueueBuffer_Count(ctypes.byref(rx_queue))
    valid_packet = False

    while (working_index < buffer_count) and (buffer_count - working_index) >= working_segment_size:
        if parser_state == 0:
            if lib.QueueBuffer_Get(ctypes.byref(rx_queue), ctypes.byref(ctypes.c_uint8())) == CRSF_HEADER:
                parser_state = 1
                working_segment_size = 2
                working_index = 0
                buffer_count = lib.QueueBuffer_Count(ctypes.byref(rx_queue))
                continue
            else:
                parser_statistics.contents.disposed_bytes += 1

            working_index = 0
            working_segment_size = 1

        elif parser_state == 1:
            packet_size = c_uint8()
            packet_type = c_uint8()
            lib.QueueBuffer_Peek(ctypes.byref(rx_queue), working_index, ctypes.byref(packet_size))
            lib.QueueBuffer_Peek(ctypes.byref(rx_queue), working_index + 1, ctypes.byref(packet_type))

            working_descriptor = FindCrsfDescriptor(packet_type.value)

            if working_descriptor is not None:
                if packet_size.value != working_descriptor.packet_size + 2:
                    parser_statistics.contents.invalid_known_packet_sizes += 1
                    parser_state = 0
                    working_segment_size = 1
                    working_index = 0
                    buffer_count = lib.QueueBuffer_Count(ctypes.byref(rx_queue))
                    continue

                working_segment_size = working_descriptor.packet_size

            else:
                working_segment_size = packet_size.value - 2

                if working_index + working_segment_size + 1 > CRSF_MAX_PACKET_LEN:
                    parser_statistics.contents.invalid_unknown_packet_sizes += 1
                    parser_state = 0
                    working_segment_size = 1
                    working_index = 0
                    buffer_count = lib.QueueBuffer_Count(ctypes.byref(rx_queue))
                    continue

            parser_state = 2

        elif parser_state == 2:
            working_index += working_segment_size
            working_segment_size = 1
            parser_state = 3

        elif parser_state == 3:
            lib.QueueBuffer_PeekBuffer(ctypes.byref(rx_queue), 0, process_buffer, working_index + 1)

            if lib.Crc8Calc(process_buffer + 2, working_index - 2) == process_buffer[working_index]:
                if working_descriptor is not None:
                    if working_descriptor.processor is not None:
                        if working_descriptor.processor(process_buffer + 2, working_index - 2, new_packet):
                            parser_statistics.contents.crcs_valid_known_packets += 1
                            valid_packet = True
                else:
                    parser_statistics.contents.crcs_valid_unknown_packets += 1

                lib.QueueBuffer_Dequeue(ctypes.byref(rx_queue), working_index + 1)

            else:
                parser_statistics.contents.crcs_invalid += 1

            working_index = 0
            working_segment_size = 1
            parser_state = 0

            if valid_packet:
                return True

        buffer_count = lib.QueueBuffer_Count(ctypes.byref(rx_queue))

    return False