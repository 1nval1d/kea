// Copyright (C) 2012  Internet Systems Consortium, Inc. ("ISC")
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

#ifndef __LABELSEQUENCE_H
#define __LABELSEQUENCE_H 1

#include <dns/name.h>
#include <util/buffer.h>

namespace isc {
namespace dns {

/// \brief Light-weight Accessor to Name data.
///
/// The purpose of this class is to easily match Names and parts of Names,
/// without needing to copy the underlying data on each label strip.
///
/// It can only work on existing Name objects, or data as provided by the
/// Name object or another LabelSequence, and the data or Name MUST
/// remain in scope during the entire lifetime of its associated
/// LabelSequence(s).
///
/// Upon creation of a LabelSequence, it records the offsets of the
/// labels in the wireformat data of the Name. When stripLeft() or
/// stripRight() is called on the LabelSequence, no changes in the
/// original data occur, but the internal pointers of the
/// LabelSequence are modified.
///
/// LabelSequences can be compared to other LabelSequences, and their
/// data can be requested (which then points to part of the original
/// data of the original Name object).
class LabelSequence {
    // Name calls the private toText(bool) method of LabelSequence.
    friend std::string Name::toText(bool) const;

public:
    /// \brief Constructs a LabelSequence for the given name
    ///
    /// \note The associated Name MUST remain in scope during the lifetime
    /// of this LabelSequence, since getData() refers to data from the
    /// Name object (the only data the LabelSequence stores are pointers
    /// to the labels in the Name object).
    ///
    /// \param name The Name to construct a LabelSequence for
    explicit LabelSequence(const Name& name):
        data_(&name.ndata_[0]),
        offsets_(&name.offsets_[0]),
        first_label_(0),
        last_label_(name.getLabelCount() - 1)
    {}

    /// \brief Constructor from serialized image.
    ///
    /// This constructor restores a \c LabelSequence object from a serialized
    /// binary image previously generated by \c serialize().  Any other input
    /// to this constructor will result in undefined behavior.
    ///
    /// The binary data passed to this constructor MUST remain in scope and
    /// MUST NOT be modified during the lifetime of this LabelSequence.
    ///
    /// As long as the data were previously generated by a call to
    /// \c serialize() on a valid \c LabelSequence object, this constructor
    /// should succeed.  While any other case is undefined, this constructor
    /// may perform some validity checks internally for safety.  Nevertheless,
    /// applications must not rely on such checks.
    ///
    /// \param buf Pointer to the serialized image generated by \c serialize().
    explicit LabelSequence(const void* buf);

    /// \brief Return the wire-format data for this LabelSequence
    ///
    /// The data is returned as a pointer to (the part of) the original
    /// wireformat data, from either the original Name object, or the
    /// raw data given in the constructor, and the given len value is
    /// set to the number of octets that match this labelsequence.
    ///
    /// \note The data pointed to is only valid if the original Name
    /// object or data is still in scope
    ///
    /// \param len Pointer to a size_t where the length of the data
    ///        will be stored (in number of octets)
    /// \return Pointer to the wire-format data of this label sequence
    const uint8_t* getData(size_t* len) const;

    /// \brief Return the length of the wire-format data of this LabelSequence
    ///
    /// This method returns the number of octets for the data that would
    /// be returned by the \c getData() method.
    ///
    /// Note that the return value of this method is always positive.
    /// Note also that if the return value of this method is 1, it means the
    /// sequence consists of the null label, i.e., a single "dot", and vice
    /// versa.
    ///
    /// \note The data pointed to is only valid if the original Name
    /// object or data is still in scope
    ///
    /// \return The length of the data of the label sequence in octets.
    size_t getDataLength() const;

    /// \brief Max possible size of serialized image generated by \c serialize
    ///
    /// A fixed length buffer of this size can be always passed to
    /// \c serialize() safely.  (But the application shouldn't use the
    /// specific size value; it must use this constant variable).
    static const size_t MAX_SERIALIZED_LENGTH =
        Name::MAX_WIRE + Name::MAX_LABELS + 1;

    /// \brief Return the size of serialized image of the \c LabelSequence.
    ///
    /// This method calculates the size of necessary storage to store
    /// serialized image of this \c LabelSequence (which would be dumped by
    /// \c serialize()) and returns it.  The size is in bytes.
    ///
    /// \throw none.
    ///
    /// \return The size of serialized image of the \c LabelSequence.
    size_t getSerializedLength() const;

    /// \brief Serialize the \c LabelSequence object in to a buffer.
    ///
    /// This method dumps a serialized image of this \c LabelSequence
    /// that would be restored by the corresponding constructor into the
    /// given buffer.  The buffer size must be at least equal to
    /// the value returned by getSerializedLength() (it can be larger than
    /// that).
    ///
    /// The serialized image would be as follows:
    /// - olen: number of offsets (1 byte)
    /// - binary sequence of offsets (olen bytes, verbatim copy of offsets_
    ///   of this size)
    /// - binary sequence of name data (length determined by itself, verbatim
    ///   copy of data_ of the corresponding size)
    ///
    /// Applications must use the resulting image opaque value and must not
    /// use it for other purposes than input to the corresponding constructor
    /// to restore it.  Application behavior that assumes the specific
    /// organization of the image is not guaranteed.
    ///
    /// \throw isc::BadValue buf_len is too short (this method never throws
    /// otherwise)
    ///
    /// \param buf Pointer to the placeholder to dump the serialized image
    /// \param buf_len The size of available region in \c buf
    void serialize(void* buf, size_t buf_len) const;

    /// \brief Compares two label sequences for equality.
    ///
    /// Performs a (optionally case-insensitive) comparison between this
    /// LabelSequence and another LabelSequence for equality.
    ///
    /// \param other The LabelSequence to compare with
    /// \param case_sensitive If true, comparison is case-insensitive
    /// \return true if The label sequences consist are the same length,
    ///         and contain the same data.
    bool equals(const LabelSequence& other, bool case_sensitive = false) const;

    /// \brief Compares two label sequences.
    ///
    /// Performs a (optionally case-insensitive) comparison between this
    /// LabelSequence and another LabelSequence.
    ///
    /// \param other The LabelSequence to compare with
    /// \param case_sensitive If true, comparison is case-insensitive
    /// \return a <code>NameComparisonResult</code> object representing the
    /// comparison result.
    NameComparisonResult compare(const LabelSequence& other,
                                 bool case_sensitive = false) const;

    /// \brief Remove labels from the front of this LabelSequence
    ///
    /// \note No actual memory is changed, this operation merely updates the
    /// internal pointers based on the offsets in the Name object.
    ///
    /// \exception OutOfRange if i is greater than or equal to the number
    ///           of labels currently pointed to by this LabelSequence
    ///
    /// \param i The number of labels to remove.
    void stripLeft(size_t i);

    /// \brief Remove labels from the end of this LabelSequence
    ///
    /// \note No actual memory is changed, this operation merely updates the
    /// internal pointers based on the offsets originally provided.
    ///
    /// \exception OutOfRange if i is greater than or equal to the number
    ///           of labels currently pointed to by this LabelSequence
    ///
    /// \param i The number of labels to remove.
    void stripRight(size_t i);

    /// \brief Returns the current number of labels for this LabelSequence
    ///
    /// \return The number of labels
    size_t getLabelCount() const {
        return (last_label_ - first_label_ + 1);
    }

    /// \brief Convert the LabelSequence to a string.
    ///
    /// This method returns a <code>std::string</code> object representing the
    /// LabelSequence as a string.  The returned string ends with a dot
    /// '.' if the label sequence is absolute.
    ///
    /// This function assumes the underlying data is in proper
    /// uncompressed wire format.  If it finds an unexpected label
    /// character including compression pointer, an exception of class
    /// \c BadLabelType will be thrown.  In addition, if resource
    /// allocation for the result string fails, a corresponding standard
    /// exception will be thrown.
    ///
    /// \return a string representation of the <code>LabelSequence</code>.
    std::string toText() const;

private:
    /// \brief Convert the LabelSequence to a string.
    ///
    /// This method is a version of the zero-argument toText() method,
    /// that accepts a <code>omit_final_dot</code> argument. The
    /// returned string ends with a dot '.' if
    /// <code>omit_final_dot</code> is <code>false</code>.
    ///
    /// This method is used as a helper for <code>Name::toText()</code>
    /// only.
    ///
    /// \param omit_final_dot whether to omit the trailing dot in the output.
    /// \return a string representation of the <code>LabelSequence</code>.
    std::string toText(bool omit_final_dot) const;

public:
    /// \brief Calculate a simple hash for the label sequence.
    ///
    /// This method calculates a hash value for the label sequence as binary
    /// data.  If \c case_sensitive is false, it ignores the case stored in
    /// the labels; specifically, it normalizes the labels by converting all
    /// upper case characters to lower case ones and calculates the hash value
    /// for the result.
    ///
    /// This method is intended to provide a lightweight way to store a
    /// relatively small number of label sequences in a hash table.
    /// For this reason it only takes into account data up to 16 octets
    /// (16 was derived from BIND 9's implementation).  Also, the function does
    /// not provide any unpredictability; a specific sequence will always have
    /// the same hash value.  It should therefore not be used in the context
    /// where an untrusted third party can mount a denial of service attack by
    /// forcing the application to create a very large number of label
    /// sequences that have the same hash value and expected to be stored in
    /// a hash table.
    ///
    /// \exception None
    ///
    /// \param case_sensitive
    /// \return A hash value for this label sequence.
    size_t getHash(bool case_sensitive) const;

    /// \brief Checks whether the label sequence is absolute
    ///
    /// \return true if the last label is the root label
    bool isAbsolute() const;

private:
    const uint8_t* data_;       // wire-format name data
    const uint8_t* offsets_;    // an array of offsets in data_ for the labels
    size_t first_label_;        // index of offsets_ for the first label
    size_t last_label_;         // index of offsets_ for the last label.
                                // can be equal to first_label_, but must not
                                // be smaller (the class ensures that)
};


///
/// \brief Insert the label sequence as a string into stream.
///
/// This method convert the \c label_sequence into a string and inserts
/// it into the output stream \c os.
///
/// This function overloads the global operator<< to behave as described in
/// ostream::operator<< but applied to \c LabelSequence objects.
///
/// \param os A \c std::ostream object on which the insertion operation is
/// performed.
/// \param label_sequence The \c LabelSequence object output by the operation.
/// \return A reference to the same \c std::ostream object referenced by
/// parameter \c os after the insertion operation.
std::ostream&
operator<<(std::ostream& os, const LabelSequence& label_sequence);

} // end namespace dns
} // end namespace isc

#endif

// Local Variables:
// mode: c++
// End:
