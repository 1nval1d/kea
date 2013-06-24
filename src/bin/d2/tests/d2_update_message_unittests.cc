// Copyright (C) 2012-2013 Internet Systems Consortium, Inc. ("ISC")
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

#include <config.h>

#include <d2/d2_update_message.h>
#include <dns/messagerenderer.h>
#include <dns/rdataclass.h>
#include <dns/rdata.h>
#include <dns/rrttl.h>

#include <boost/scoped_ptr.hpp>
#include <gtest/gtest.h>

using namespace std;
using namespace isc;
using namespace isc::d2;
using namespace isc::dns;
using namespace isc::dns::rdata;
using namespace isc::util;

namespace {

class D2UpdateMessageTest : public ::testing::Test {
public:
    D2UpdateMessageTest() {
    }

    ~D2UpdateMessageTest() {
    };

    // @brief Return string representation of the name encoded in wire format.
    //
    // This function reads the number of bytes specified in the second
    // argument from the buffer. It doesn't check if buffer has sufficient
    // length for reading given number of bytes. Caller should verify it
    // prior to calling this function.
    //
    // @param buf input buffer, its internal pointer will be moved to
    //        the position after a name being read from it.
    // @param name_length length of the name stored in the buffer
    // @param no_zero_byte if true it indicates that the given buffer does not
    //        comprise the zero byte, which signals end of the name. This is
    //        the case, when dealing with compressed messages which don't have
    //        this byte.
    //
    // @return string representation of the name.
    std::string readNameFromWire(InputBuffer& buf, size_t name_length,
                                 bool no_zero_byte = false) {
        // 64 characters bytes should be sufficent for current tests.
        // It may be extended if required.
        char name_data[64];
        // Create another InputBuffer which holds only the name in the wire
        // format.
        buf.readData(name_data, name_length);
        if (no_zero_byte) {
            ++name_length;
            name_data[name_length-1] = 0;
        }
        InputBuffer name_buf(name_data, name_length);
        // Parse the name and return its textual representation.
        Name name(name_buf);
        return (name.toText());
    }
};

// This test verifies that the DNS message is properly decoded from the
// wire format.
TEST_F(D2UpdateMessageTest, fromWire) {
    // The following table holds the DNS response in on-wire format.
    // This message comprises the following sections:
    // - HEADER
    // - PREREQUISITE section with one RR
    // - UPDATE section with 1 RR.
    // Such a response may be generated by the DNS server as a result
    // of copying the contents of the REQUEST message sent by DDNS client.
    const uint8_t bin_msg[] = {
        // HEADER section starts here (see RFC 2136, section 2).
        0x05, 0xAF, // ID=0x05AF
        0xA8, 0x6,  // QR=1, Opcode=6, RCODE=YXDOMAIN
        0x0, 0x1,   // ZOCOUNT=1
        0x0, 0x2,   // PRCOUNT=2
        0x0, 0x1,   // UPCOUNT=1
        0x0, 0x0,   // ADCOUNT=0

        // Zone section starts here. The The first field comprises
        // the Zone name encoded as a set of labels, each preceded
        // by a length of the following label. The whole Zone name is
        // terminated with a NULL char.
        // For Zone section format see (RFC 2136, section 2.3).
        0x7, 0x65, 0x78, 0x61, 0x6D, 0x70, 0x6C, 0x65, // example (0x7 is a length)
        0x3, 0x63, 0x6F, 0x6D, //.com. (0x3 is a length)
        0x0,      // NULL character terminates the Zone name.
        0x0, 0x6, // ZTYPE='SOA'
        0x0, 0x1, // ZCLASS='IN'

        // Prerequisite section starts here. This section comprises two
        // prerequisites:
        // - 'Name is not in use'
        // - 'Name is in use'
        // See RFC 2136, section 2.4 for the format of Prerequisite section.
        // Each prerequisite RR starts with its name. It is expressed in the
        // compressed format as described in RFC 1035, section 4.1.4. The first
        // label is expressed as in case of non-compressed name. It is preceded
        // by the length value. The following two bytes are the pointer to the
        // offset in the message where 'example.com' was used. That is, in the
        // Zone name at offset 12. Pointer starts with two bits set - they
        // mark start of the pointer.

        // First prerequisite. NONE class indicates that the update requires
        // that the name 'foo.example.com' is not use/
        0x03, 0x66, 0x6F, 0x6F, // foo.
        0xC0, 0x0C,             // pointer to example.com.
        0x0, 0x1C,              // TYPE=AAAA
        0x0, 0xFE,              // CLASS=NONE
        0x0, 0x0, 0x0, 0x0,     // TTL=0
        0x0, 0x0,               // RDLENGTH=0

        // Second prerequisite. ANY class indicates tha the update requires
        // that the name 'bar.example.com' exists.
        0x03, 0x62, 0x61, 0x72, // bar.
        0xC0, 0x0C,             // pointer to example.com.
        0x0, 0x1C,              // TYPE=AAAA
        0x0, 0xFF,              // CLASS=ANY
        0x0, 0x0, 0x0, 0x0,     // TTL=0
        0x0, 0x0,               // RDLENGTH=0

        // Update section starts here. The format of this section conforms to
        // RFC 2136, section 2.5. The name of the RR is again expressed in
        // compressed format. The two pointer bytes point to the offset in the
        // message where 'foo.example.com' was used already - 29.
        0xC0, 0x1D,             // pointer to foo.example.com.
        0x0, 0x1C,              // TYPE=AAAA
        0x0, 0x1,               // CLASS=IN
        0xAA, 0xBB, 0xCC, 0xDD, // TTL=0xAABBCCDD
        0x0, 0x10,              // RDLENGTH=16
        // The following 16 bytes of RDATA hold IPv6 address: 2001:db8:1::1.
        0x20, 0x01, 0x0D, 0xB8, 0x00, 0x01, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01
    };
    InputBuffer buf(bin_msg, sizeof(bin_msg));

    D2UpdateMessage msg(true);

    ASSERT_NO_THROW(msg.fromWire(buf));

    EXPECT_EQ(0x05AF, msg.getQid());
    EXPECT_EQ(D2UpdateMessage::RESPONSE, msg.getQRFlag());
    EXPECT_EQ(Rcode::YXDOMAIN_CODE, msg.getRcode().getCode());
    ASSERT_EQ(1, msg.getRRCount(D2UpdateMessage::SECTION_ZONE));
    // Zone section is TBD

    ASSERT_EQ(2, msg.getRRCount(D2UpdateMessage::SECTION_PREREQUISITE));
    RRsetIterator rrset_it = msg.beginSection(D2UpdateMessage::SECTION_PREREQUISITE);
    RRsetPtr prereq1 = *rrset_it;
    ASSERT_TRUE(prereq1);
    EXPECT_EQ("foo.example.com.", prereq1->getName().toText());
    EXPECT_EQ(RRType::AAAA().getCode(), prereq1->getType().getCode());
    EXPECT_EQ(RRClass::NONE().getCode(), prereq1->getClass().getCode());
    EXPECT_EQ(0, prereq1->getTTL().getValue());
    EXPECT_EQ(0, prereq1->getRdataCount());

    // Move to next prerequisite section.
    ++rrset_it;

    RRsetPtr prereq2 = *rrset_it;
    ASSERT_TRUE(prereq2);
    EXPECT_EQ("bar.example.com.", prereq2->getName().toText());
    EXPECT_EQ(RRType::AAAA().getCode(), prereq2->getType().getCode());
    EXPECT_EQ(RRClass::ANY().getCode(), prereq2->getClass().getCode());
    EXPECT_EQ(0, prereq2->getTTL().getValue());
    EXPECT_EQ(0, prereq2->getRdataCount());

    ASSERT_EQ(1, msg.getRRCount(D2UpdateMessage::SECTION_UPDATE));
    rrset_it = msg.beginSection(D2UpdateMessage::SECTION_UPDATE);
    RRsetPtr update = *rrset_it;
    ASSERT_TRUE(update);
    EXPECT_EQ("foo.example.com.", update->getName().toText());
    EXPECT_EQ(RRType::AAAA().getCode(), update->getType().getCode());
    EXPECT_EQ(RRClass::IN().getCode(), update->getClass().getCode());
    EXPECT_EQ(0xAABBCCDD, update->getTTL().getValue());
    ASSERT_EQ(1, update->getRdataCount());
    RdataIteratorPtr rdata_it = update->getRdataIterator();
    ASSERT_TRUE(rdata_it);
    in::AAAA rdata_ref("2001:db8:1::1");
    EXPECT_EQ(0, rdata_ref.compare(rdata_it->getCurrent()));
}

// This test verifies that the wire format of the message is produced
// in the render mode.
TEST_F(D2UpdateMessageTest, toWire) {
    D2UpdateMessage msg;
    // Set message ID.
    msg.setQid(0x1234);
    // Make it a Request message by setting the QR flag to 0.
    msg.setQRFlag(D2UpdateMessage::REQUEST);
    // Rcode to NOERROR.
    msg.setRcode(Rcode(Rcode::NOERROR_CODE));

    // Set Zone section. This section must comprise exactly
    // one Zone. toWire function would fail if Zone is not set.
    msg.setZone(Name("example.com"), RRClass::IN());

    // Set prerequisities.

    // 'Name Is Not In Use' prerequisite (RFC 2136, section 2.4.5)
    RRsetPtr prereq1(new RRset(Name("foo.example.com"), RRClass::NONE(),
                               RRType::ANY(), RRTTL(0)));
    msg.addRRset(D2UpdateMessage::SECTION_PREREQUISITE, prereq1);

    // 'Name is In Use' prerequisite (RFC 2136, section 2.4.4)
    RRsetPtr prereq2(new RRset(Name("bar.example.com"), RRClass::ANY(),
                               RRType::ANY(), RRTTL(0)));
    msg.addRRset(D2UpdateMessage::SECTION_PREREQUISITE, prereq2);

    // Set Update Section.

    // Create RR holding a name being added. This RR is constructed
    // in conformance to RFC 2136, section 2.5.1.
    RRsetPtr updaterr1(new RRset(Name("foo.example.com"), RRClass::IN(),
                                 RRType::A(), RRTTL(10)));
    // RR record is of the type A, thus RDATA holds 4 octet Internet
    // address. This address is 10.10.1.1.
    char rdata1[] = {
        0xA, 0xA , 0x1, 0x1
    };
    InputBuffer buf_rdata1(rdata1, 4);
    updaterr1->addRdata(createRdata(RRType::A(), RRClass::IN(), buf_rdata1,
                                    buf_rdata1.getLength()));
    // Add the RR to the message.
    msg.addRRset(D2UpdateMessage::SECTION_UPDATE, updaterr1);

    // Render message into the wire format.
    MessageRenderer renderer;
    ASSERT_NO_THROW(msg.toWire(renderer));

    // Make sure that created packet is not truncated.
    ASSERT_EQ(77, renderer.getLength());

    // Create input buffer from the rendered data. InputBuffer
    // is handy to validate the byte contents of the rendered
    // message.
    InputBuffer buf(renderer.getData(), renderer.getLength());

    // Start validating the message header.

    // Verify message ID.
    EXPECT_EQ(0x1234, buf.readUint16());
    // The 2-bytes following message ID comprise the following fields:
    // - QR - 1 bit indicating that it is REQUEST. Should be 0.
    // - Opcode - 4 bits which should hold value of 5 indicating this is
    //            an Update message. Binary form is "0101".
    // - Z - These bits are unused for Update Message and should be 0.
    // - RCODE - Response code, set to NOERROR for REQUEST. It is 0.
    //8706391835
    // The binary value is:
    //   0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
    // +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
    // | QR|     Opcode    |           Z               |     RCODE     |
    // +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
    // | 0 | 0   1   0   1 | 0   0   0   0   0   0   0 | 0   0   0   0 |
    // +---+---+---+-------+---+---+---+---+---+---+---+---+---+---+---+
    // and the hexadecimal representation is 0x2800.
    EXPECT_EQ(0x2800, buf.readUint16());

    // ZOCOUNT - holds the number of zones for the update. For Request
    // message it must be exactly one record (RFC2136, section 2.3).
    EXPECT_EQ(1, buf.readUint16());

    // PRCOUNT - holds the number of prerequisites. Earlier we have added
    // two prerequisites. Thus, expect that this conter is 2.
    EXPECT_EQ(2, buf.readUint16());

    // UPCOUNT - holds the number of RRs in the Update Section. We have
    // added 1 RR, which adds the name foo.example.com to the Zone.
    EXPECT_EQ(1, buf.readUint16());

    // ADCOUNT - holds the number of RRs in the Additional Data Section.
    EXPECT_EQ(0, buf.readUint16());

    // Start validating the Zone section. This section comprises the
    // following data:
    // - ZNAME
    // - ZTYPE
    // - ZCLASS

    // ZNAME holds 'example.com.' encoded as set of labels. Each label
    // is preceded by its length. The name is ended with the byte holding
    // zero value. This yields the total size of the name in wire format
    // of 13 bytes.

    // The simplest way to convert the name from wire format to a string
    // is to use dns::Name class. It should be ok to rely on the Name class
    // to decode the name, because it is unit tested elswhere.
    std::string zone_name = readNameFromWire(buf, 13);
    EXPECT_EQ("example.com.", zone_name);

    // ZTYPE of the Zone section must be SOA according to RFC 2136,
    // section 2.3.
    EXPECT_EQ(RRType::SOA().getCode(), buf.readUint16());

    // ZCLASS of the Zone section is IN.
    EXPECT_EQ(RRClass::IN().getCode(), buf.readUint16());

    // Start checks on Prerequisite section. Each prerequisite comprises
    // the following fields:
    // - NAME - name of the RR in wire format
    // - TYPE - two octets with one of the RR TYPE codes
    // - CLASS - two octets with one of the RR CLASS codes
    // - TTL - a 32-bit signed integer specifying Time-To-Live
    // - RDLENGTH - length of the RDATA field
    // - RDATA - a variable length string of octets containing
    //           resource data.
    // In case of this message, we expect to have two prerequisite RRs.
    // Their structure is checked below.

    // First prerequisite should comprise the 'Name is not in use prerequisite'
    // for 'foo.example.com'.

    // Check the name first. Message renderer is using compression for domain
    // names as described in RFC 1035, section 4.1.4. The name in this RR is
    // foo.example.com. The name of the zone is example.com and it has occured
    // in this message already at offset 12 (the size of the header is 12).
    // Therefore, name of this RR is encoded as 'foo', followed by a pointer
    // to offset in this message where the remainder of this name was used.
    // This pointer has the following format:
    // +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    // | 1  1|                 OFFSET                  |
    // +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    // | 1  1| 0  0  0  0  0  0  0  0  0  0  1  1  0  0|
    // +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    // which has a following hexadecimal representation: 0xC00C

    // Let's read the non-compressed part first - 'foo.'
    std::string name_prereq1 = readNameFromWire(buf, 4, true);
    EXPECT_EQ("foo.", name_prereq1);
    // The remaining two bytes hold the pointer to 'example.com'.
    EXPECT_EQ(0xC00C, buf.readUint16());
    // TYPE is ANY
    EXPECT_EQ(RRType::ANY().getCode(), buf.readUint16());
    // CLASS is NONE
    EXPECT_EQ(RRClass::NONE().getCode(), buf.readUint16());
    // TTL is a 32-but value, expecting 0
    EXPECT_EQ(0, buf.readUint32());
    // There is no RDATA, so RDLENGTH is 0
    EXPECT_EQ(0, buf.readUint16());

    // Start checking second prerequisite.

    std::string name_prereq2 = readNameFromWire(buf, 4, true);
    EXPECT_EQ("bar.", name_prereq2);
    // The remaining two bytes hold the pointer to 'example.com'.
    EXPECT_EQ(0xC00C, buf.readUint16());
    // TYPE is ANY
    EXPECT_EQ(RRType::ANY().getCode(), buf.readUint16());
    // CLASS is ANY
    EXPECT_EQ(RRClass::ANY().getCode(), buf.readUint16());
    // TTL is a 32-but value, expecting 0
    EXPECT_EQ(0, buf.readUint32());
    // There is no RDATA, so RDLENGTH is 0
    EXPECT_EQ(0, buf.readUint16());

    // Start checking Update section. This section contains RRset with
    // one A RR.

    // The name of the RR is 'foo.example.com'. It is encoded in the
    // compressed format - as a pointer to the name of prerequisite 1.
    // This name is in offset 0x1D in this message.
    EXPECT_EQ(0xC01D, buf.readUint16());
    // TYPE is A
    EXPECT_EQ(RRType::A().getCode(), buf.readUint16());
    // CLASS is IN (same as zone class)
    EXPECT_EQ(RRClass::IN().getCode(), buf.readUint16());
    // TTL is a 32-but value, set here to 10.
    EXPECT_EQ(10, buf.readUint32());
    // For A records, the RDATA comprises the 4-byte Internet address.
    // So, RDLENGTH is 4.
    EXPECT_EQ(4, buf.readUint16());
    // We have stored the following address in RDATA field: 10.10.1.1
    // (which is 0A 0A 01 01) in hexadecimal format.
    EXPECT_EQ(0x0A0A0101, buf.readUint32());

    // @todo: consider extending this test to verify Additional Data
    // section.
}

} // End of anonymous namespace
