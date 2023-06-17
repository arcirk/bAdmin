#ifndef CRYPTER_HPP
#define CRYPTER_HPP
#pragma once

#pragma warning( disable: 4275 )
#pragma warning( disable: 4819 )

#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/archive/iterators/binary_from_base64.hpp>

//#include <iostream>

//#include <cryptopp/dll.h>
//#include <cryptopp/cryptlib.h>
//#include <cryptopp/aes.h>
//#include <cryptopp/filters.h>
//#include <cryptopp/modes.h>

//using namespace std;
#include <cryptopp/osrng.h>
using CryptoPP::AutoSeededRandomPool;

#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

#include <string>
using std::string;

#include <cstdlib>
using std::exit;

#include <cryptopp/cryptlib.h>
using CryptoPP::Exception;

#include <cryptopp/hex.h>
using CryptoPP::HexEncoder;
using CryptoPP::HexDecoder;

#include <cryptopp/filters.h>
using CryptoPP::StringSink;
using CryptoPP::StringSource;
using CryptoPP::StreamTransformationFilter;

#include <cryptopp/aes.h>
using CryptoPP::AES;

#include <cryptopp/modes.h>
using CryptoPP::ECB_Mode;

USING_NAMESPACE(CryptoPP)

namespace arcirk::cryptography {

    const std::string base64_padding[] = {"", "==", "="};

    inline std::string base64_encode(const std::string &s) {
        namespace bai = boost::archive::iterators;

        try {
            std::stringstream os;

            // convert binary values to base64 characters
            typedef bai::base64_from_binary
            // retrieve 6 bit integers from a sequence of 8 bit bytes
            <bai::transform_width<const char *, 6, 8>> base64_enc; // compose all the above operations in to a new iterator

            std::copy(base64_enc(s.c_str()), base64_enc(s.c_str() + s.size()),
                      std::ostream_iterator<char>(os));

            os << base64_padding[s.size() % 3];
            return os.str();
        }catch (std::exception& e){
            std::cerr << "error: " << e.what() << std::endl;
            return "";
        }

    }

    inline std::string base64_decode(const std::string &s) {
        namespace bai = boost::archive::iterators;

        try {
            std::stringstream os;

            typedef bai::transform_width<bai::binary_from_base64<const char *>, 8, 6> base64_dec;

            auto size = (unsigned int)s.size();

            // Remove the padding characters, cf. https://svn.boost.org/trac/boost/ticket/5629
            if (size && s[size - 1] == '=') {
                --size;
                if (size && s[size - 1] == '=') --size;
            }
            if (size == 0) return {};

            std::copy(base64_dec(s.data()), base64_dec(s.data() + size),
                      std::ostream_iterator<char>(os));

            return os.str();

        }catch (std::exception& e){
            std::cerr << "error: " << e.what() << std::endl;
            return "";
        }
    }

class crypt_utils
{
//public:
//    enum MODE
//    {
//        AES,
//        DES
//    };

public:
    //crypt_utils(MODE mode = MODE::AES);
    crypt_utils();

//    int get_filesize(const char *filename);

//    int save(const std::string& filepath, const std::string& str);
//    std::string load(const std::string& filepath);

    std::string encrypt_string(const std::string& plain_text);
    std::string decrypt_string(const std::string& cipher_text);

//    std::string encrypt_file(const std::string& infile);
//    std::string decrypt_file(const std::string& infile);

//    int encrypt_file(const std::string& infile, const std::string& outfile);
//    int decrypt_file(const std::string& infile, const std::string& outfile);
//protected:
//    std::string tohex(const char& c);
//    std::string tohexs(const std::string& str);

//    char tochar(const char& hex1, const char& hex2);
//    std::string tochars(const std::string& hex);

private:
    // 32 + 16
    const byte aes_key[CryptoPP::AES::MAX_KEYLENGTH] = {
        0x01,0x23,0x45,0x67,0x89,0xab,0xcd,0xef,
        0x01,0x23,0x45,0x67,0x89,0xab,0xcd,0xef,
        0x01,0x23,0x45,0x67,0x89,0xab,0xcd,0xef,
        0x01,0x23,0x45,0x67,0x89,0xab,0xcd,0xef
    };
    const byte aes_iv[CryptoPP::AES::BLOCKSIZE] = {
        0x12,0x34,0x56,0x78,0x90,0xab,0xcd,0xef,
        0x12,0x34,0x56,0x78,0x90,0xab,0xcd,0xef
    };

//    // 24+8
//    const byte des_key[CryptoPP::DES_EDE3::MAX_KEYLENGTH] = {
//        0x01,0x23,0x45,0x67,0x89,0xab,0xcd,0xef,
//        0x01,0x23,0x45,0x67,0x89,0xab,0xcd,0xef,
//        0x01,0x23,0x45,0x67,0x89,0xab,0xcd,0xef
//    };
//    const byte des_iv[CryptoPP::DES_EDE3::BLOCKSIZE] = {
//        0x12,0x34,0x56,0x78,0x90,0xab,0xcd,0xef
//    };

    CryptoPP::CFB_FIPS_Mode<CryptoPP::AES>::Encryption aes_encryption;
    CryptoPP::CFB_FIPS_Mode<CryptoPP::AES>::Decryption aes_decryption;

//    CryptoPP::CFB_FIPS_Mode<CryptoPP::DES_EDE3>::Encryption des_encryption;
//    CryptoPP::CFB_FIPS_Mode<CryptoPP::DES_EDE3>::Decryption des_decryption;

   //MODE m_mode;
};

} // namespace arcirk::cryptography

#endif // CRYPTER_HPP
