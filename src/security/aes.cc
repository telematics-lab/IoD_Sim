#include "aes.h"

namespace ns3
{

/**
 * @brief AES constructor that takes in a key and an Initialisation Vector (IV).
 * @param key The key used for encryption/decryption.
 * @param iv The IV used for encryption/decryption.
 */
AES::AES(const unsigned char* key, const unsigned char* iv)
{
    this->key = new unsigned char[KEY_LENGTH + 1];
    this->iv = new unsigned char[IV_LENGTH + 1];
    set_key(key);
    set_iv(iv);
}

/**
 * @brief Sets the AES key.
 * @param key The 256 bits key to set.
 * @throws std::invalid_argument if the key length is not 256 bits.
 */
void
AES::set_key(const unsigned char* key)
{
    if (strlen((const char*)key) != KEY_LENGTH)
        throw std::invalid_argument("The key must be 256 bits!");
    strcpy((char*)this->key, (char*)key);
}

/**
 * @brief Gets the AES key.
 * @return A pointer to the class's key member variable.
 */
unsigned char*
AES::get_key() const
{
    return this->key;
}

/**
 * @brief Sets the AES IV.
 * @param iv The 128 bits IV to set.
 * @throws std::invalid_argument if the IV is not 128 bits.
 */
void
AES::set_iv(const unsigned char* iv)
{
    if (strlen((const char*)iv) != IV_LENGTH)
        throw std::invalid_argument("The IV must be 128 bits!");
    strcpy((char*)this->iv, (char*)iv);
}

/**
 * @brief Gets the AES IV.
 * @return A pointer to the class's IV member variable.
 */
unsigned char*
AES::get_iv() const
{
    return iv;
}

/**
 * @brief Encrypts plaintext using AES-256-CBC and stores the result in ciphertext.
 * @param plaintext The plaintext to encrypt.
 * @param ciphertext The buffer to store the encrypted ciphertext in.
 * @return The length of the encrypted ciphertext or an error code.
 */
int
AES::encrypt(const unsigned char* plaintext, char unsigned* ciphertext)
{
    int plaintext_length = strlen((const char*)plaintext);
    int ciphertext_length = 0;
    int length = 0;

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();

    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv) != ERR_LIB_NONE)
    {
        std::cout << "EVP_EncryptInit_ex failed" << std::endl;
        return ERROR;
    }

    ciphertext_length += length;

    if (EVP_EncryptUpdate(ctx, ciphertext, &length, plaintext, plaintext_length) != ERR_LIB_NONE)
    {
        std::cout << "EVP_EncryptUpdate failed" << std::endl;
        return ERROR;
    }

    ciphertext_length += length;

    if (EVP_EncryptFinal_ex(ctx, ciphertext + ciphertext_length, &length) != ERR_LIB_NONE)
    {
        std::cout << "EVP_EncryptFinal_ex failed" << std::endl;
        return ERROR;
    }

    ciphertext_length += length;

    EVP_CIPHER_CTX_free(ctx);

    return ciphertext_length;
}

/**
 * @brief Decrypts ciphertext using AES-256-CBC and stores the result in plaintext.
 * @param ciphertext The ciphertext to decrypt.
 * @param plaintext The buffer to store the decrypted plaintext in.
 * @return The length of the decrypted plaintext or an error code.
 */
int
AES::decrypt(const unsigned char* ciphertext, unsigned char* plaintext)
{
    int ciphertext_length = strlen((const char*)ciphertext);
    int plaintext_length = 0;
    int length = 0;

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    EVP_CIPHER_CTX_set_padding(ctx, 0);

    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv) != ERR_LIB_NONE)
    {
        std::cout << "EVP_DecryptInit_ex failed" << std::endl;
        return ERROR;
    }

    plaintext_length += length;

    if (EVP_DecryptUpdate(ctx, plaintext, &length, ciphertext, ciphertext_length) != ERR_LIB_NONE)
    {
        std::cout << "EVP_DecryptUpdate failed" << std::endl;
        return ERROR;
    }

    plaintext_length += length;

    EVP_CIPHER_CTX_free(ctx);

    return plaintext_length;
}

/**
 * @brief Prints the given ciphertext.
 * @param ciphertext The ciphertext to print.
 */
void
AES::print_ciphertext(const unsigned char* ciphertext)
{
    printf("Ciphertext:\n");
    BIO_dump_fp(stdout, ciphertext, strlen((char*)ciphertext));
}

/**
 * @brief Destructor for the AES class.
 */
AES::~AES()
{
    delete[] key;
    delete[] iv;
}

} // namespace ns3