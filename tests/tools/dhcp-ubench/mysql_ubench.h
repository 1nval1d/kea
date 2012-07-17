// Copyright (C) 2012 Internet Systems Consortium, Inc. ("ISC")
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

#include <string>
#include "benchmark.h"

class MySQL_uBenchmark: public uBenchmark {
public:
    MySQL_uBenchmark(const std::string& hostname, const std::string& user,
                     const std::string& pass, const std::string& db,
                     uint32_t num_iterations, bool sync,
                     bool verbose);

    virtual void printInfo();
    virtual void connect();
    virtual void disconnect();
    virtual void createLease4Test();
    virtual void searchLease4Test();
    virtual void updateLease4Test();
    virtual void deleteLease4Test();

protected:
    void failure(const char* operation);

    MYSQL * Conn_;
};
