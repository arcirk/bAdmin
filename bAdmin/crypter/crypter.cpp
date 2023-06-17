#include "crypter/crypter.hpp"
#include <sys/stat.h>
#include "shared_struct.hpp"

namespace arcirk::cryptography {

crypt_utils::crypt_utils()
{
    //m_mode = mode;

//    // for des
//    des_encryption.SetKeyWithIV(des_key, sizeof(des_key), des_iv);
//    des_decryption.SetKeyWithIV(des_key, sizeof(des_key), des_iv);

    // for aes
    aes_encryption.SetKeyWithIV(aes_key, sizeof(aes_key), aes_iv);
    aes_decryption.SetKeyWithIV(aes_key, sizeof(aes_key), aes_iv);
}

//#pragma region hex/char

//std::string crypt_utils::tohex(const char& c)
//{
//    /*
//    00,01,02,...0F   [0,1,2,...15]
//    10,11,12,...1F   [16,17,...31]
//    ...
//    F0,F1,F2,...FF    [240,241,...255]
//    */

//    /*
//    char ch[2] = { 0 };
//    sprintf(ch, "%02x", static_cast<byte>(c)); //  char ---> 16 hex
//    return std::string(ch);
//    */
//    const char HEX[] = "0123456789ABCDEF";
//    const int HEX_BASE = 16;

//    std::string c_hex;

//    int uc = (unsigned char)(c);
//    int a = uc / HEX_BASE;
//    int b = uc % HEX_BASE;
//    c_hex.push_back(HEX[a]);
//    c_hex.push_back(HEX[b]);

//    return c_hex;
//}

//std::string crypt_utils::tohexs(const std::string& str)
//{
//    const char HEX[] = "0123456789ABCDEF";
//    const int HEX_BASE = 16;

//    std::string hex_result;
//    for (size_t i = 0; i < str.size(); i++)
//    {
//        std::string c_hex;
//        int uc = (unsigned char)(str[i]);
//        int a = uc / HEX_BASE;
//        int b = uc % HEX_BASE;
//        c_hex.push_back(HEX[a]);
//        c_hex.push_back(HEX[b]);

//        hex_result += c_hex;
//    }
//    return hex_result;
//}

//char crypt_utils::tochar(const char& hex1, const char& hex2)
//{
//    const int HEX_BASE = 16;

//    int h1 = (int)hex1;
//    int h2 = (int)hex2;

//    // F1 ===> 16,1===>15*16+1 = 241 ===> char
//    char uc = (char)(h1*HEX_BASE + h2);
//    return uc;
//}

//std::string crypt_utils::tochars(const std::string& hex)
//{
//    if (hex.size() % 2 != 0)
//    {
//        return "";
//    };

//    std::map<char, int> hex_int = {
//        { '0' , 0 },
//        { '1', 1 },
//        { '2', 2 },
//        { '3', 3 },
//        { '4', 4 },
//        { '5', 5 },
//        { '6', 6 },
//        { '7', 7 },
//        { '8', 8 },
//        { '9', 9 },
//        { 'A', 10 },
//        { 'B', 11 },
//        { 'C', 12 },
//        { 'D', 13 },
//        { 'E', 14 },
//        { 'F', 15 },
//    };

//    std::string char_result;
//    const int HEX_BASE = 16;

//    for (size_t i = 0; i < hex.size(); i += 2)
//    {
//        int h1 = hex_int.at(hex[i]);   // 'F' ===>15
//        int h2 = hex_int.at(hex[i + 1]); // '1'===>1

//                                         // F1 ===> 15,1===>15*16+1 = 241 ===> char
//                                         //std::cout << "===============\n";
//        char uc = (char)(h1*HEX_BASE + h2);
//        //std::cout << h1 <<","<<h2 << std::endl;
//        //std::cout << uc << std::endl;
//        char_result += uc;
//    }
//    return char_result;
//}
//#pragma endregion

//#pragma region encrypt_string and decrypt_string

//int crypt_utils::get_filesize(const char *filename)
//{
//    struct stat f_stat;
//    if (stat(filename, &f_stat) == -1)
//    {
//        return -1;
//    }
//    return f_stat.st_size;
//}

/*
std::string load_text_not_use(const std::string& filepath)
{
std::string str;
ifstream in(filepath, ios::in | ios::binary);

std::string line;
std::string content;
while (getline(in, line))
{
//std::cout << "line.size()=" << line.size() << std::endl;
if (line.size() > 0) {
content += line + "\n";
}
line.clear();
}

in.close();

return content;
}
*/

//int crypt_utils::save(const std::string& filepath, const std::string& str)
//{
//    std::ofstream out(filepath, std::ios::out | std::ios::binary); // binary mode (default text mode)
//    out.write(str.c_str(), str.length());
//    out.close();
//    return 1;
//}

//std::string crypt_utils::load(const std::string& filepath)
//{
//    int filesize = get_filesize(filepath.c_str());

//    char *buffer = new char[filesize];

//    std::ifstream in(filepath, std::ios::in | std::ios::binary); // binary mode (default text mode)
//    in.read(buffer, filesize);
//    in.close();

//    // std::string content(buffer); // ERROR for binary file
//    std::string content;
//    for (size_t i = 0; i < filesize; i++)
//    {
//        content.push_back(buffer[i]);
//    }
//    delete[] buffer;

//    return content;
//}

/*
//==========================================================================
std::string cipher_text;
CryptoPP::AES::CipherApiImpl aesEncryption(key, CryptoPP::AES::MAX_KEYLENGTH);
CryptoPP::CBC_Mode_ExternalCipher::CipherApiImpl cbcEncryption(aesEncryption, iv);
CryptoPP::StreamTransformationFilter stfEncryptor(cbcEncryption, new CryptoPP::StringSink(cipher_text));
stfEncryptor.Put(reinterpret_cast<const unsigned char*>(plain_text.c_str()), plain_text.length());
stfEncryptor.MessageEnd();

std::string cipher_text_hex = tohexs(cipher_text);
return cipher_text_hex;
//==========================================================================
CryptoPP::AES::Decryption aesDecryption(key, CryptoPP::AES::MAX_KEYLENGTH);
CryptoPP::CBC_Mode_ExternalCipher::Decryption cbcDecryption(aesDecryption, iv);
CryptoPP::StreamTransformationFilter stfDecryptor(cbcDecryption, new CryptoPP::StringSink(decryptedText));
stfDecryptor.Put(reinterpret_cast<const unsigned char*>(cipher_text.c_str()), cipher_text.size());
stfDecryptor.MessageEnd();

return decrypted_text;
//==========================================================================

byte ciphertext[24];
byte decrypted[24];

CFB_FIPS_Mode<DES_EDE3>::Encryption encryption_DES_EDE3_CFB;
encryption_DES_EDE3_CFB.SetKeyWithIV(key, sizeof(key), iv);
encryption_DES_EDE3_CFB.ProcessString(ciphertext, plaintext, 24);

CFB_FIPS_Mode<DES_EDE3>::Decryption decryption_DES_EDE3_CFB;
decryption_DES_EDE3_CFB.SetKeyWithIV(key, sizeof(key), iv);
decryption_DES_EDE3_CFB.ProcessString(decrypted, ciphertext, 24);
*/

std::string crypt_utils::encrypt_string(const std::string& plain_text)
{
//    size_t size = plain_text.size();
//    const byte* in_byte = (const unsigned char*)(plain_text.c_str());

//    byte* out_byte = new byte[size];

//    des_encryption.ProcessString(out_byte, in_byte, size);

//    std::string cipher_text;
//    for (size_t i = 0; i < size; i++)
//    {
//        cipher_text.push_back(out_byte[i]);
//    }
//    delete[] out_byte;

    std::string plain = arcirk::from_utf(plain_text);

    string cipher;

    try
    {
        StringSource s(plain, true,
            new StreamTransformationFilter(aes_encryption,
                new StringSink(cipher)
            ) // StreamTransformationFilter
        ); // StringSource

#if 0
        StreamTransformationFilter filter(e);
        filter.Put((const byte*)plain.data(), plain.size());
        filter.MessageEnd();

        const size_t ret = filter.MaxRetrievable();
        cipher.resize(ret);
        filter.Get((byte*)cipher.data(), cipher.size());
#endif
    }
    catch (const CryptoPP::Exception& e)
    {
        cerr << e.what() << endl;
        exit(1);
    }

    return arcirk::to_utf(base64_encode(cipher));
}

std::string crypt_utils::decrypt_string(const std::string& cipher_text)
{
//    size_t size = cipher_text.size();
//    const byte* in_byte = (const unsigned char*)(cipher_text.c_str());
//    byte* out_byte = new byte[size];

//    des_decryption.ProcessString(out_byte, in_byte, size);

//    std::string plain_text;
//    for (size_t i = 0; i < size; i++)
//    {
//        plain_text.push_back(out_byte[i]);
//    }
//    delete[] out_byte;

//    return plain_text;


    std::string cipher = base64_decode(cipher_text);

    string recovered;
    try
        {
            StringSource s(cipher, true,
                new StreamTransformationFilter(aes_decryption,
                    new StringSink(recovered)
                ) // StreamTransformationFilter
            ); // StringSource

    #if 0
            StreamTransformationFilter filter(d);
            filter.Put((const byte*)cipher.data(), cipher.size());
            filter.MessageEnd();

            const size_t ret = filter.MaxRetrievable();
            recovered.resize(ret);
            filter.Get((byte*)recovered.data(), recovered.size());
    #endif

            //cout << "recovered text: " << recovered << endl;
        }
        catch (const CryptoPP::Exception& e)
        {
            cerr << e.what() << endl;
            exit(1);
        }

    return arcirk::to_utf(recovered);
}

//std::string crypt_utils::encrypt_file(const std::string& infile)
//{
//    std::string plain_text = load(infile);
//    std::string cipher_text = encrypt_string(plain_text);
//    return cipher_text;
//}

//std::string crypt_utils::decrypt_file(const std::string& infile)
//{
//    std::string cipher_text = load(infile);
//    std::string plain_text = decrypt_string(cipher_text);
//    return plain_text;
//}

//int crypt_utils::encrypt_file(const std::string& infile, const std::string& outfile)
//{
//    std::string cipher_text_hex = encrypt_file(infile);
//    return save(outfile, cipher_text_hex);
//}

//int crypt_utils::decrypt_file(const std::string& infile, const std::string& outfile)
//{
//    std::string plain_text = decrypt_file(infile);
//    return save(outfile, plain_text);
//}

//#pragma endregion

} // namespace arcirk::cryptography
