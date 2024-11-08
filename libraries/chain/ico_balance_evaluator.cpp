/*
 * AcloudBank
 *
 */
#include <ctime>
#include <iomanip>

#include <graphene/chain/ico_balance_evaluator.hpp>
#include <graphene/protocol/pts_address.hpp>
#include <graphene/tokendistribution/tokendistribution.hpp>

namespace graphene { namespace chain {

void_result ico_balance_claim_evaluator::do_evaluate(const ico_balance_claim_operation& op)
{
   database& d = db();

   ico_balance = &op.balance_to_claim(d);
   if( !ico_balance ) return {};

   const account_object* account = d.find(fc::variant(op.deposit_to_account, 1).as<account_id_type>(1));

   // Build the verification phrase
   std::time_t tm = time_t( d.head_block_time().sec_since_epoch() );
   std::string datetime(11,0);
   datetime.resize(std::strftime(&datetime[0], datetime.size(), "%Y-%m-%d", std::localtime(&tm)));

   using namespace std::string_literals;
   std::string msg = "I "s + account->name + " want to claim "s + GRAPHENE_SYMBOL + " tokens. "s + datetime + "."s;

   FC_ASSERT(tokendistribution::verifyMessage(op.eth_pub_key, msg, op.eth_sign) == 0, "The key or the signature is not correct");
   FC_ASSERT(ico_balance->eth_address == tokendistribution::getAddress(op.eth_pub_key));

   //FC_ASSERT(op.total_claimed.asset_id == ico_balance->asset_type());

   return {};
}

/**
 * @note the fee is always 0 for this particular operation because once the
 * balance is claimed it frees up memory and it cannot be used to spam the network
 */
void_result ico_balance_claim_evaluator::do_apply(const ico_balance_claim_operation& op)
{
   database& d = db();

   if( ico_balance )
   {
      d.adjust_balance(op.deposit_to_account, ico_balance->balance);
   }
   d.remove(*ico_balance);

   return {};
}
} } // namespace graphene::chain
