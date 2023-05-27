#ifndef AES_H
#define AES_H

#include <iostream>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <string.h>

#define KEY_LENGTH 32
#define IV_LENGTH 16
#define ERROR -1

namespace ns3
{

class AES
{
  private:
    unsigned char* key;
    unsigned char* iv;

  public:
    AES(const unsigned char* key, const unsigned char* iv);
    void set_key(const unsigned char* key);
    unsigned char* get_key() const;
    void set_iv(const unsigned char* iv);
    unsigned char* get_iv() const;
    int encrypt(const unsigned char* plaintext, unsigned char* ciphertext);
    int decrypt(const unsigned char* ciphertext, unsigned char* plaintext);
    void print_ciphertext(const unsigned char* ciphertext);
    ~AES();
};

} // namespace ns3

#endif