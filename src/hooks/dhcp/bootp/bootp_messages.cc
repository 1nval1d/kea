// File created from ../../../../src/hooks/dhcp/bootp/bootp_messages.mes on Mon Jan 20 2020 17:15

#include <cstddef>
#include <log/message_types.h>
#include <log/message_initializer.h>

extern const isc::log::MessageID BOOTP_BOOTP_QUERY = "BOOTP_BOOTP_QUERY";
extern const isc::log::MessageID BOOTP_LOAD = "BOOTP_LOAD";
extern const isc::log::MessageID BOOTP_PACKET_OPTIONS_SKIPPED = "BOOTP_PACKET_OPTIONS_SKIPPED";
extern const isc::log::MessageID BOOTP_PACKET_UNPACK_FAILED = "BOOTP_PACKET_UNPACK_FAILED";
extern const isc::log::MessageID BOOTP_UNLOAD = "BOOTP_UNLOAD";

namespace {

const char* values[] = {
    "BOOTP_BOOTP_QUERY", "recognized a BOOTP query: %1",
    "BOOTP_LOAD", "Bootp hooks library has been loaded",
    "BOOTP_PACKET_OPTIONS_SKIPPED", "an error upacking an option, caused subsequent options to be skipped: %1",
    "BOOTP_PACKET_UNPACK_FAILED", "failed to parse query from %1 to %2, received over interface %3, reason: %4",
    "BOOTP_UNLOAD", "Bootp hooks library has been unloaded",
    NULL
};

const isc::log::MessageInitializer initializer(values);

} // Anonymous namespace

