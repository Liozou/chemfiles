// Chemfiles, a modern library for chemistry file reading and writing
// Copyright (C) Guillaume Fraux and contributors -- BSD license

#include "catch.hpp"
#include "helpers.hpp"
#include "chemfiles/files/XzFile.hpp"
#include "chemfiles/Error.hpp"
#include <fstream>
using namespace chemfiles;

TEST_CASE("Read a text file") {
    SECTION("Standard read") {
        auto file = TextFile("data/xyz/water.xyz.xz", File::READ, File::LZMA);

        CHECK(file.readline() == "297");
        CHECK(file.readline() == " generated by VMD");
        CHECK(file.readline() == "  O          0.417219        8.303366       11.737172");

        file.rewind();
        CHECK(file.readline() == "297");
        CHECK(file.readline() == " generated by VMD");

        // Count lines
        file.rewind();
        size_t lines = 0;
        while (!file.eof()) {
            file.readline();
            lines++;
        }

        CHECK(lines == 29901);
        CHECK(file.tellpos() == 1606000);
        CHECK(file.eof());

        file.seekpos(23804);
        CHECK(file.readline() == "  H          8.479585        0.521128       11.514298");
    }

    SECTION("Constructor errors") {
        CHECK_THROWS_WITH(
            XzFile("not existing", File::READ),
            "could not open the file at 'not existing'"
        );

        CHECK_THROWS_WITH(
            XzFile("data/xyz/water.xyz.xz", File::APPEND),
            "appending (open mode 'a') is not supported with xz files"
        );
    }

    SECTION("Lines offsets") {
        // Compare offset with uncompressed file
        auto file = TextFile("data/xyz/water.xyz", File::READ, File::DEFAULT);
        auto positions = std::vector<uint64_t>();
        while (!file.eof()) {
            positions.push_back(file.tellpos());
            file.readline();
        }

        auto gz_file = TextFile("data/xyz/water.xyz.xz", File::READ, File::LZMA);
        for (size_t i=0; i<positions.size(); i++) {
            CHECK(positions[i] == gz_file.tellpos());
            gz_file.readline();
        }
        CHECK(gz_file.eof());
    }
}

TEST_CASE("Write an xz file") {
    auto filename = NamedTempPath(".xz");

    {
        auto file = TextFile(filename, File::WRITE, File::LZMA);
        file.print("Test\n");
        file.print("{}\n", 5467);
    }

    std::ifstream verification(filename, std::ios::binary);
    REQUIRE(verification.is_open());
    verification.seekg(0, std::ios::end);
    auto size = static_cast<size_t>(verification.tellg());
    verification.seekg(0, std::ios::beg);

    auto content = std::vector<uint8_t>(size);
    verification.read(reinterpret_cast<char*>(content.data()), static_cast<std::streamsize>(size));

    auto expected = std::vector<uint8_t> {
        0xfd, 0x37, 0x7a, 0x58, 0x5a, 0x00, 0x00, 0x04, 0xe6, 0xd6, 0xb4, 0x46,
        0x02, 0x00, 0x21, 0x01, 0x16, 0x00, 0x00, 0x00, 0x74, 0x2f, 0xe5, 0xa3,
        0x01, 0x00, 0x09, 0x54, 0x65, 0x73, 0x74, 0x0a, 0x35, 0x34, 0x36, 0x37,
        0x0a, 0x00, 0x00, 0x00, 0xbd, 0xb5, 0x7a, 0x14, 0x41, 0x54, 0x79, 0xbe,
        0x00, 0x01, 0x22, 0x0a, 0x15, 0x1a, 0xe1, 0x67, 0x1f, 0xb6, 0xf3, 0x7d,
        0x01, 0x00, 0x00, 0x00, 0x00, 0x04, 0x59, 0x5a
    };
    CHECK(content == expected);
}

TEST_CASE("In-memory decompression") {
    auto content = std::vector<uint8_t> {
        0xfd, 0x37, 0x7a, 0x58, 0x5a, 0x00, 0x00, 0x04, 0xe6, 0xd6, 0xb4, 0x46,
        0x02, 0x00, 0x21, 0x01, 0x16, 0x00, 0x00, 0x00, 0x74, 0x2f, 0xe5, 0xa3,
        0x01, 0x00, 0x09, 0x54, 0x65, 0x73, 0x74, 0x0a, 0x35, 0x34, 0x36, 0x37,
        0x0a, 0x00, 0x00, 0x00, 0xbd, 0xb5, 0x7a, 0x14, 0x41, 0x54, 0x79, 0xbe,
        0x00, 0x01, 0x22, 0x0a, 0x15, 0x1a, 0xe1, 0x67, 0x1f, 0xb6, 0xf3, 0x7d,
        0x01, 0x00, 0x00, 0x00, 0x00, 0x04, 0x59, 0x5a
    };

    auto decompressed = xzinflate_in_place(reinterpret_cast<const char*>(content.data()), content.size());
    CHECK(std::string(decompressed.begin(), decompressed.end()) == "Test\n5467\n");

    content[23] = 0x00;
    CHECK_THROWS_WITH(
        xzinflate_in_place(reinterpret_cast<const char*>(content.data()), content.size()),
        "lzma: compressed file is corrupted (code: 9)"
    );

    content[0] = 0x00;
    CHECK_THROWS_WITH(
        xzinflate_in_place(reinterpret_cast<const char*>(content.data()), content.size()),
        "lzma: input not in .xz format (code: 7)"
    );
}
