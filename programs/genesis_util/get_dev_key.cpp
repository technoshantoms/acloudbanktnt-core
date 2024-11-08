/*
 * AcloudBank
 *
 */

#include <iostream>
#include <string>

#include <fc/crypto/elliptic.hpp>
#include <fc/crypto/base58.hpp>
#include <fc/io/json.hpp>

#include <graphene/protocol/address.hpp>
#include <graphene/protocol/types.hpp>
#include <graphene/utilities/key_conversion.hpp>

#ifndef WIN32
#include <csignal>
#endif

using namespace std;

int main( int argc, char** argv )
{
   try
   {
      if( argc == 2 ) {
         fc::limited_mutable_variant_object mvo(5);
         graphene::protocol::public_key_type pub_key(argv[1]);
         mvo( "public_key", std::string( pub_key ) )
            ( "address", graphene::protocol::address( pub_key ) )
            ;
         std::cout << fc::json::to_string( fc::mutable_variant_object(mvo) );

         return 0;
      }

      std::string dev_key_prefix;
      bool need_help = false;
      if( argc < 3 ) // requires at least a prefix and a suffix
         need_help = true;
      else
      {
         dev_key_prefix = argv[1];
         if(  (dev_key_prefix == "-h")
           || (dev_key_prefix == "--help")
           )
           need_help = true;
      }

      if( need_help )
      {
         std::cerr << "\nThis program generates keys with specified prefix and suffix(es) as seed(s).\n\n"
             "Syntax:\n\n"
             "  get_dev_key <prefix> <suffix> ...\n\n"
             "Examples:\n\n"
             "  get_dev_key nath an\n"
             "  get_dev_key wxyz- owner-5 active-7 balance-9 wit-block-signing-3 wit-owner-5 wit-active-33\n"
             "  get_dev_key wxyz- wit-block-signing-0:101\n"
             "\n";
         return 1;
      }

      bool comma = false;

      auto show_key = [&comma]( const fc::ecc::private_key& priv_key )
      {
         fc::limited_mutable_variant_object mvo(5);
         graphene::protocol::public_key_type pub_key = priv_key.get_public_key();
         mvo( "private_key", graphene::utilities::key_to_wif( priv_key ) )
            ( "public_key", std::string( pub_key ) )
            ( "address", graphene::protocol::address( pub_key ) )
            ;
         if( comma )
            std::cout << ",\n";
         std::cout << fc::json::to_string( fc::mutable_variant_object(mvo) );
         comma = true;
      };

      std::cout << "[";

      for( int i=2; i<argc; i++ )
      {
         std::string arg = argv[i];
         std::string prefix;
         int lep = -1, rep = -1;
         auto dash_pos = arg.rfind('-');
         if( dash_pos != string::npos )
         {
            std::string lhs = arg.substr( 0, dash_pos+1 );
            std::string rhs = arg.substr( dash_pos+1 );
            auto colon_pos = rhs.find(':');
            if( colon_pos != string::npos )
            {
               prefix = lhs;
               lep = std::stoi( rhs.substr( 0, colon_pos ) );
               rep = std::stoi( rhs.substr( colon_pos+1 ) );
            }
         }
         if( lep >= 0 )
         {
            for( int k=lep; k<rep; k++ )
            {
               std::string s = dev_key_prefix + prefix + std::to_string(k);
               show_key( fc::ecc::private_key::regenerate( fc::sha256::hash( s ) ) );
            }
         }
         else
         {
            show_key( fc::ecc::private_key::regenerate( fc::sha256::hash( dev_key_prefix + arg ) ) );
         }
      }
      std::cout << "]\n";
   }
   catch ( const fc::exception& e )
   {
      std::cerr << e.to_detail_string() << "\n";
      return 1;
   }
   return 0;
}
