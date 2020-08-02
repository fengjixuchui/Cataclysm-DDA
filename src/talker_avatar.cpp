#include "avatar.h"
#include "game.h"
#include "game_constants.h"
#include "messages.h"
#include "monster.h"
#include "mtype.h"
#include "npc.h"
#include "npctrade.h"
#include "output.h"
#include "player.h"
#include "talker_avatar.h"

static const efftype_id effect_pacified( "pacified" );
static const efftype_id effect_pet( "pet" );

static const bionic_id bio_armor_eyes( "bio_armor_eyes" );
static const bionic_id bio_deformity( "bio_deformity" );
static const bionic_id bio_face_mask( "bio_face_mask" );
static const bionic_id bio_voice( "bio_voice" );

static const trait_id trait_PROF_FOODP( "PROF_FOODP" );

std::vector<std::string> talker_avatar::get_topics( bool )
{
    std::vector<std::string> add_topics;
    if( has_trait( trait_PROF_FOODP ) && !( is_wearing( itype_id( "foodperson_mask" ) ) ||
                                            is_wearing( itype_id( "foodperson_mask_on" ) ) ) ) {
        add_topics.push_back( "TALK_NOFACE" );
    }
    return add_topics;
}

int talker_avatar::parse_mod( const std::string &attribute, const int factor ) const
{
    int modifier = 0;
    if( attribute == "U_INTIMIDATE" ) {
        modifier = me_chr->intimidation();
    }
    modifier *= factor;
    return modifier;
}

int talker_avatar::trial_chance_mod( const std::string &trial_type ) const
{
    int chance = 0;
    const social_modifiers &me_mods = me_chr->get_mutation_social_mods();
    if( trial_type == "lie" ) {
        chance += me_chr->talk_skill() + me_mods.lie;

        //come on, who would suspect a robot of lying?
        if( me_chr->has_bionic( bio_voice ) ) {
            chance += 10;
        }
        if( me_chr->has_bionic( bio_face_mask ) ) {
            chance += 20;
        }
    } else if( trial_type == "persuade" ) {
        chance += me_chr->talk_skill() + me_mods.persuade;

        if( me_chr->has_bionic( bio_face_mask ) ) {
            chance += 10;
        }
        if( me_chr->has_bionic( bio_deformity ) ) {
            chance -= 50;
        }
        if( me_chr->has_bionic( bio_voice ) ) {
            chance -= 20;
        }
    } else if( trial_type == "intimidate" ) {
        chance += me_chr->intimidation() + me_mods.intimidate;

        if( me_chr->has_bionic( bio_face_mask ) ) {
            chance += 10;
        }
        if( me_chr->has_bionic( bio_armor_eyes ) ) {
            chance += 10;
        }
        if( me_chr->has_bionic( bio_deformity ) ) {
            chance += 20;
        }
        if( me_chr->has_bionic( bio_voice ) ) {
            chance += 20;
        }
    }
    return chance;
}

void talker_avatar::buy_monster( talker &seller, const mtype_id &mtype, int cost,
                                 int count, bool pacified, const translation &name )
{
    npc *seller_guy = seller.get_npc();
    if( !seller_guy ) {
        popup( _( "%s can't sell you any %s" ), seller.disp_name(), mtype.obj().nname( 2 ) );
        return;
    }
    if( !npc_trading::pay_npc( *seller_guy, cost ) ) {
        popup( _( "You can't afford it!" ) );
        return;
    }

    for( int i = 0; i < count; i++ ) {
        monster *const mon_ptr = g->place_critter_around( mtype, me_chr->pos(), 3 );
        if( !mon_ptr ) {
            add_msg( m_debug, "Cannot place u_buy_monster, no valid placement locations." );
            break;
        }
        monster &tmp = *mon_ptr;
        // Our monster is always a pet.
        tmp.friendly = -1;
        tmp.add_effect( effect_pet, 1_turns, true );

        if( pacified ) {
            tmp.add_effect( effect_pacified, 1_turns, true );
        }

        if( !name.empty() ) {
            tmp.unique_name = name.translated();
        }

    }

    if( name.empty() ) {
        popup( _( "%1$s gives you %2$d %3$s." ), seller_guy->name,
               count, mtype.obj().nname( count ) );
    } else {
        popup( _( "%1$s gives you %2$s." ), seller_guy->name, name );
    }
}
