#include <catch.hpp>

#include <cstdint>
#include <fstream>
#include <istream>
#include <streambuf>

// create synthetic data

// save a .mashiro binary file

// load a .mashiro binary file and make sure the data is correct with the source

TEST_CASE("File Format Test", "[Mashiro]") {

    std::filebuf file;
    REQUIRE(file.open("simple.mashiro", std::ios_base::out | std::ios_base::binary));

    std::streamsize offset;

    constexpr uint16_t BOM = 0xFEFF;
    constexpr uint16_t file_version = 1;
    constexpr size_t file_len = 8;
    constexpr char file_type[file_len] = "mashiro";

    constexpr uint32_t header_size = 0;

    offset = file.sputn(reinterpret_cast<const char *>(&BOM), sizeof(BOM));
    offset = file.sputn(file_type, file_len);
    offset = file.sputn(reinterpret_cast<const char *>(&file_version), sizeof(file_version));
    offset = file.pubseekoff(sizeof(uint64_t), std::ios_base::cur);
    const auto file_size_pos = offset;
    offset = file.sputn(reinterpret_cast<const char *>(&header_size), sizeof(header_size));
    const auto file_size = static_cast<uint64_t>(offset);

    file.pubseekpos(file_size_pos);
    file.sputn(reinterpret_cast<const char *>(&file_size), sizeof(file_size));
}