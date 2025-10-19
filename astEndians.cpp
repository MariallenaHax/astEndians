#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <cstring>
bool isLE = false;
int countnyan = 0;
#pragma pack(push, 1)
struct astHeader {
    char magic[4];     
    uint32_t size;
    uint16_t audio;
    uint16_t bitDepth;
    uint16_t channels;
    uint16_t unknown4;
    uint32_t sampleRate;
    uint32_t totalSamples;
    uint32_t loopStart;
    uint32_t loopEnd;        
    uint32_t unknown;
    uint32_t unknown2;
    uint32_t unknown3;
    char magic2[20];
};
#pragma pack(pop)
#pragma pack(push, 1)
struct astBLCKHeader {
    uint32_t string;
    uint32_t size;
    char magic2[24];
};
#pragma pack(pop)
uint32_t Swap32(uint32_t val) {
    return ((val >> 24) & 0x000000FF) |
           ((val >> 8)  & 0x0000FF00) |
           ((val << 8)  & 0x00FF0000) |
           ((val << 24) & 0xFF000000);
}
uint16_t Swap16(uint16_t val) {
    return (val >> 8) | (val << 8);
}
int CountBLCKSections(std::ifstream& in, bool isLE, uint32_t astSize) {
    int count = 0;
    std::streampos start = in.tellg();

    while (in && in.tellg() < static_cast<std::streampos>(astSize)) {
        std::streampos pos = in.tellg();

        astBLCKHeader header;
        in.read(reinterpret_cast<char*>(&header), sizeof(header));
        if (in.gcount() < sizeof(header)) break;

        char magic[4];
        in.read(magic, 4);
        in.seekg(-4, std::ios::cur);

        std::string magicStr(magic, 4);
        if (magicStr == "BLCK") {
           in.seekg(pos);
            break;
        }

        uint32_t dataSize = isLE ? Swap32(header.size) : header.size;
        in.seekg(dataSize, std::ios::cur); 
        count++;
    }

    in.clear();
    in.seekg(start);
    return count;
}


void CopyRawBlock(std::ifstream& in, std::ofstream& out, uint32_t offset, uint32_t endOffset) {
    const size_t bufferSize = 4096;
    char buffer[bufferSize];

    in.seekg(offset);
    out.seekp(offset);

    uint32_t remaining = endOffset - offset;
    while (remaining > 0) {
        size_t chunk = std::min(static_cast<uint32_t>(bufferSize), remaining);
        in.read(buffer, chunk);
        out.write(buffer, chunk);
        remaining -= chunk;
    }
}
uint32_t ConvertAst(std::ifstream& in, std::ofstream& out) {
    astHeader ast;
    in.read(reinterpret_cast<char*>(&ast), sizeof(ast));
uint32_t base2 = static_cast<uint32_t>(in.tellg());
    std::reverse(ast.magic, ast.magic + 4);
    ast.size = Swap32(ast.size);
    ast.bitDepth = Swap16(ast.bitDepth);
    ast.channels = Swap16(ast.channels);
    ast.unknown4 = Swap16(ast.unknown4);
    ast.sampleRate = Swap32(ast.sampleRate);
    ast.totalSamples = Swap32(ast.totalSamples);
    ast.loopStart = Swap32(ast.loopStart);
    ast.loopEnd = Swap32(ast.loopEnd);
    ast.unknown = Swap32(ast.unknown);
    out.write(reinterpret_cast<char*>(&ast), sizeof(ast));

if (isLE == false)
{
    
while (in && in.peek() != EOF) {
    uint32_t base = static_cast<uint32_t>(in.tellg());
    astBLCKHeader node;
    in.read(reinterpret_cast<char*>(&node), sizeof(node));
    node.string = Swap32(node.string);
    node.size = Swap32(node.size);
    std::vector<char> soundData(node.size * ast.channels);
    in.read(soundData.data(), node.size * ast.channels);
    out.write(reinterpret_cast<char*>(&node), sizeof(node));
    out.write(soundData.data(), node.size * ast.channels);
}


}
else
{
while (in && in.peek() != EOF) {
    uint32_t base = static_cast<uint32_t>(in.tellg());
    astBLCKHeader node;
    in.read(reinterpret_cast<char*>(&node), sizeof(node));
    node.string = Swap32(node.string);
    node.size = Swap32(node.size);
    std::vector<char> soundData(Swap32(node.size) * Swap16(ast.channels));
    in.read(soundData.data(), Swap32(node.size) * Swap16(ast.channels));
    out.write(reinterpret_cast<char*>(&node), sizeof(node));
    out.write(soundData.data(), Swap32(node.size) * Swap16(ast.channels));
}
}
    return 0;
}

int main(int argc, char* argv[]) {
    std::string inputPath  = argv[1];
    std::string  outputPath;
    if (!argv[2])
    {
outputPath = inputPath + "_out.ast";
    }
    else
{
outputPath = argv[2];
}
if (argc < 2) {
        std::cerr << "Usage: astEndians.exe <input.ast> (output.ast)\n";
        return 1;
    }
    std::ifstream in(inputPath, std::ios::binary);
    std::ofstream out(outputPath, std::ios::binary);

    if (!in || !out) {
        std::cerr << "404 file not found.\n";
        return 1;
    }
    while (in.peek() != EOF) {
        char magic[4];
        in.read(magic, 4);
        in.seekg(-4, std::ios::cur);

        std::string magicStr(magic, 4);
        if (magicStr == "MRTS") {
            isLE = true;
            ConvertAst(in, out);
        }
        else if (magicStr == "STRM") {
            ConvertAst(in, out);
        }
    }
    std::cout << "DONE\n";
    return 0;
}