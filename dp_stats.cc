#include "htslibpp.h"
#include "htslibpp_proxies.h"
#include <iostream>
#include <algorithm>

using namespace YiCppLib::HTSLibpp;

int main(int argc, char* argv[]) {

    auto fileHandle = htsOpen(argc < 2 ? "-" : argv[1], "r");
    if(not fileHandle) {
        std::cerr<<"unable to open file"<<std::endl;
        return 1;
    }

    auto header = htsHeader<bcfHeader>::read(fileHandle);

    auto hasDP = std::any_of(
            htsHeader<bcfHeader>::dictBegin(header, htsHeader<bcfHeader>::DictType::ID),
            htsHeader<bcfHeader>::dictEnd(header, htsHeader<bcfHeader>::DictType::ID),
            [](const auto& p) {
                auto proxy = htsProxy(p);
                return proxy.hasValueForLineType(htsHeader<bcfHeader>::LineType::INFO) && (strcmp(proxy.key(), "DP") == 0);
            });
    std::cout<<hasDP<<std::endl;

    if(not hasDP) {
        std::cerr<<"The vcf file has no DP field in its INFO dictionary"<<std::endl;
        return 2;
    }

    int32_t *dp = (int32_t *)calloc(1, sizeof(int32_t));
    int dp_size = 0;

    std::transform(
            htsReader<bcfRecord>::begin(fileHandle, header),
            htsReader<bcfRecord>::end(fileHandle, header), 
            std::ostream_iterator<int32_t>(std::cout, "\n"),
            [&header, &dp, &dp_size](const auto &p) {
                bcf_unpack(p.get(), BCF_UN_INFO);
                bcf_get_info_int32(header.get(), p.get(), "DP", &dp, &dp_size);
                return *dp;
            });
    free(dp);

    return 0;
}




