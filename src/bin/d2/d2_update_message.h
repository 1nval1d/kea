// Copyright (C) 2013  Internet Systems Consortium, Inc. ("ISC")
//
// Permission to use, copy, modify, and/or distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES WITH
// REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
// AND FITNESS.  IN NO EVENT SHALL ISC BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
// LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
// OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#ifndef D2_UPDATE_MESSAGE_H
#define D2_UPDATE_MESSAGE_H

#include <d2/d2_zone.h>
#include <dns/message.h>
#include <dns/name.h>
#include <dns/rcode.h>
#include <dns/rrclass.h>
#include <dns/rrset.h>

#include <map>

namespace isc {
namespace d2 {

/// @brief Exception indicating that Zone section contains invalid content.
///
/// This exception is thrown when ZONE section of the DNS Update message
/// is invalid. According to RFC2136, section 2.3, the zone section is
/// allowed to contain exactly one record. When Request message contains
/// more records or is empty, this exception is thrown.
class InvalidZoneSection : public Exception {
public:
    InvalidZoneSection(const char* file, size_t line, const char* what) :
        isc::Exception(file, line, what) {}
};

/// @brief Exception indicating that QR flag has invalid value.
///
/// This exception is thrown when QR flag has invalid value for
/// the operation performed on the particular message. For instance,
/// the QR flag must be set to indicate that the given message is
/// a RESPONSE when @c D2UpdateMessage::fromWire is performed.
/// The QR flag must be cleared when @c D2UpdateMessage::toWire
/// is executed.
class InvalidQRFlag : public Exception {
public:
    InvalidQRFlag(const char* file, size_t line, const char* what) :
        isc::Exception(file, line, what) {}
};

/// @brief Exception indicating that the parsed message is not DNS Update.
///
/// This exception is thrown when decoding the DNS message which is not
/// a DNS Update.
class NotUpdateMessage : public Exception {
public:
    NotUpdateMessage(const char* file, size_t line, const char* what) :
        isc::Exception(file, line, what) {}
};


/// @brief The @c D2UpdateMessage encapsulates a DNS Update message.
///
/// This class represents the DNS Update message. Functions exposed by this
/// class allow to specify the data sections carried by the message and create
/// an on-wire format of this message. This class is also used to decode
/// messages received from the DNS server from the on-wire format.
///
/// <b>Design choice:</b> A dedicated class has been created to encapsulate
/// DNS Update message because existing @c isc::dns::Message is designed to
/// support regular DNS messages described in RFC 1035 only. Altough DNS Update
/// has the same format, particular sections serve different purposes. In order
/// to avoid rewrite of significant portions of @c isc::dns::Message class, this
/// class is implemented in-terms-of @c Message class to reuse its functionality
/// wherever possible.
class D2UpdateMessage {
public:

    enum QRFlag {
        REQUEST,
        RESPONSE
    };

    enum UpdateMsgSection {
        SECTION_ZONE,
        SECTION_PREREQUISITE,
        SECTION_UPDATE,
        SECTION_ADDITIONAL
    };

public:
    D2UpdateMessage(const bool parse = false);

private:
    D2UpdateMessage(const D2UpdateMessage& source);
    D2UpdateMessage& operator=(const D2UpdateMessage& source);

public:

    QRFlag getQRFlag() const;

    uint16_t getId() const;

    void setId(const uint16_t qid);

    const dns::Rcode& getRcode() const;

    void setRcode(const dns::Rcode& rcode);

    unsigned int getRRCount(const UpdateMsgSection section) const;

    const dns::RRsetIterator beginSection(const UpdateMsgSection section) const;

    const dns::RRsetIterator endSection(const UpdateMsgSection section) const;

    void setZone(const dns::Name& zone, const dns::RRClass& rrclass);

    D2ZonePtr getZone() const;

    void addRRset(const UpdateMsgSection section, const dns::RRsetPtr& rrset);

    void toWire(dns::AbstractMessageRenderer& renderer);

    void fromWire(isc::util::InputBuffer& buffer);


private:

    static dns::Message::Section ddnsToDnsSection(const UpdateMsgSection section);
    void validate() const;

    dns::Message message_;
    D2ZonePtr zone_;

};

} // namespace d2
} // namespace isc

#endif // D2_UPDATE_MESSAGE_H
