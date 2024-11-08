/*
 * AcloudBank
 *
 */

#include "secp256k1.h"
#include <fc/exception/exception.hpp>

#include <graphene/tokendistribution/Keccak256.hpp>

namespace graphene { namespace tokendistribution {

void preparePubKey(std::string& pubKey) {
    if (pubKey.length() == 130)
      pubKey = pubKey.erase(0, 2); // drop "04"

   if (pubKey.length() != 128)
      FC_THROW_EXCEPTION(fc::assert_exception, "Ethereum key length is incorrect. Is it a real key?");
}

void prepareSignature(std::string& pubKey) {
    if (pubKey.length() == 132)
      pubKey = pubKey.erase(0, 2); // drop "0x"

   if (pubKey.length() != 130)
      FC_THROW_EXCEPTION(fc::assert_exception, "Ethereum signature length is incorrect. Is it a real signature?");
}

std::string getAddress(std::string pubKey)
{
   preparePubKey(pubKey);

   Bytes message = hexBytes(pubKey.c_str());
   std::uint8_t hashBuff[Keccak256::HASH_LEN];
   Keccak256::getHash(message.data(), message.size(), hashBuff);
   Bytes hash(hashBuff, hashBuff + Keccak256::HASH_LEN);

   std::string address = bytesHex(hash);
   address = address.substr(address.length() - 40); // Take the last 40 characters
   return address;
}

int verifyMessage (std::string pubKey, std::string msg, std::string sig) {
   preparePubKey(pubKey);

   // Wrap the phrase
   std::ostringstream msgToHash;
   msgToHash << '\x19' << "Ethereum Signed Message:\n"
              << msg.size() << msg;

   // Hash the phrase
   Bytes message = asciiBytes(msgToHash.str().c_str());
   std::uint8_t hashBuff[Keccak256::HASH_LEN];
   Keccak256::getHash(message.data(), message.size(), hashBuff);
   Bytes hash(hashBuff, hashBuff + Keccak256::HASH_LEN);

   // Read signature
   prepareSignature(sig);
   Bytes signature = hexBytes(sig.c_str());
   int recoveryId = static_cast<int>(signature[64]);
   if (recoveryId != 27 && recoveryId != 28)
      FC_THROW_EXCEPTION(fc::assert_exception, "Signature has unexpected value");
   recoveryId -= 27;

   // Recover public key from signature
   Bytes recKey(65, 0);
   unsigned int rk_len;
   const int compressed = 0;
   static secp256k1_context_t *ctx = secp256k1_context_create(SECP256K1_CONTEXT_VERIFY | SECP256K1_CONTEXT_SIGN );
   int r = secp256k1_ecdsa_recover_compact(ctx,
                reinterpret_cast<const unsigned char *>(hash.data()),
                reinterpret_cast<const unsigned char *>(signature.data()),
                reinterpret_cast<      unsigned char *>(recKey.data()),
                reinterpret_cast<int *>                (&rk_len),
                                                        compressed,
                                                        recoveryId);

   if (r != 1 || rk_len != 65)
      FC_THROW_EXCEPTION(fc::assert_exception, "Public key can't be recovered: incorrect signature");

   // If the recovered key matches the original one, then everything is fine
   std::string recoveredKey = bytesHex(recKey);
   preparePubKey(recoveredKey);
   return recoveredKey.compare(pubKey);
}

} }
