#pragma warning (disable : 26495)

#include "stdafx.h"
#include "RelationSettings.h"
#include <RelationDefinitions.h>

namespace Stalker::Mechanics::Relations
{    
    RelationSettings::RelationSettings()
    {
        m_scriptHandlerName             = pSettings->r_string(RELATION_SYSTEM_SECTION, "script_handler");
        m_isScriptActionHandlerEnabled  = pSettings->r_bool(RELATION_SYSTEM_SECTION, "is_script_enabled");
        m_isFriendlyFireEnabled         = pSettings->r_bool(RELATION_SYSTEM_SECTION, "friendly_fire");
        m_neutralRelationThreshold      = pSettings->r_s16(GAME_RELATIONS_SECTION, "attitude_neutal_threshold");
        m_friendRelationThreshold       = pSettings->r_s16(GAME_RELATIONS_SECTION, "attitude_friend_threshold");
        m_defaultEnemyGoodwill          = pSettings->r_s16(GAME_RELATIONS_SECTION, "goodwill_enemy");
        m_defaultNeutralGoodwill        = pSettings->r_s16(GAME_RELATIONS_SECTION, "goodwill_neutal");
        m_defaultFriendGoodwill         = pSettings->r_s16(GAME_RELATIONS_SECTION, "goodwill_friend");
        m_attackEnemyGoodwillAmount     = pSettings->r_s32(ACTIONS_POINTS_SECTION, "free_enemy_attack_goodwill");
        m_attackNeutralGoodwillAmount   = pSettings->r_s32(ACTIONS_POINTS_SECTION, "free_neutral_attack_goodwill");
        m_attackFriendGoodwillAmount    = pSettings->r_s32(ACTIONS_POINTS_SECTION, "free_friend_attack_goodwill");
        m_attackEnemyReputationAmount   = pSettings->r_s32(ACTIONS_POINTS_SECTION, "free_enemy_attack_reputation");
        m_attackNeutralReputationAmount = pSettings->r_s32(ACTIONS_POINTS_SECTION, "free_neutral_attack_reputation");
        m_attackFriendReputationAmount  = pSettings->r_s32(ACTIONS_POINTS_SECTION, "free_friend_attack_reputation");
        m_killEnemyGoodwillAmount       = pSettings->r_s32(ACTIONS_POINTS_SECTION, "enemy_kill_goodwill");
        m_killNeutralGoodwillAmount     = pSettings->r_s32(ACTIONS_POINTS_SECTION, "neutral_kill_goodwill");
        m_killFriendGoodwillAmount      = pSettings->r_s32(ACTIONS_POINTS_SECTION, "friend_kill_goodwill");
        m_killEnemyReputationAmount     = pSettings->r_s32(ACTIONS_POINTS_SECTION, "enemy_kill_reputation");
        m_killNeutralReputationAmount   = pSettings->r_s32(ACTIONS_POINTS_SECTION, "neutral_kill_reputation");
        m_killFriendReputationAmount    = pSettings->r_s32(ACTIONS_POINTS_SECTION, "friend_kill_reputation");
        m_minAttackDeltaTime            = u32(1000.f * pSettings->r_float(ACTIONS_POINTS_SECTION, "min_attack_delta_time"));
        m_attackMgrRememberTime         = u32(1000.f * pSettings->r_float(ACTIONS_POINTS_SECTION, "fight_remember_time"));
        m_helpEnemyGoodwillAmount       = pSettings->r_s32(ACTIONS_POINTS_SECTION, "enemy_fight_help_goodwill");
        m_helpNeutralGoodwillAmount     = pSettings->r_s32(ACTIONS_POINTS_SECTION, "neutral_fight_help_goodwill");
        m_helpFriendGoodwillAmount      = pSettings->r_s32(ACTIONS_POINTS_SECTION, "friend_fight_help_goodwill");
        m_helpEnemyReputationAmount     = pSettings->r_s32(ACTIONS_POINTS_SECTION, "enemy_fight_help_reputation");
        m_helpNeutralReputationAmount   = pSettings->r_s32(ACTIONS_POINTS_SECTION, "neutral_fight_help_reputation");
        m_helpFriendReputationAmount    = pSettings->r_s32(ACTIONS_POINTS_SECTION, "friend_fight_help_reputation");
        m_killCommunityGoodwillAmount   = pSettings->r_s32(ACTIONS_POINTS_SECTION, "community_member_kill_goodwill");
        m_personalGoodwillLimits        = pSettings->r_ivector2(ACTIONS_POINTS_SECTION, "personal_goodwill_limits");
        m_communityGoodwillLimits       = pSettings->r_ivector2(ACTIONS_POINTS_SECTION, "community_goodwill_limits");
        m_reputationLimits              = pSettings->r_ivector2(ACTIONS_POINTS_SECTION, "reputation_limits");
        m_rankLimits                    = pSettings->r_ivector2(ACTIONS_POINTS_SECTION, "rank_limits");
    }

    bool RelationSettings::IsScriptActionHandlerEnabled() const
    {
        return m_isScriptActionHandlerEnabled;
    }

    bool RelationSettings::IsFriendlyFireEnabled() const
    {
        return m_isFriendlyFireEnabled;
    }

    std::string_view RelationSettings::ScriptActionHandlerName() const
    {
        return m_scriptHandlerName.c_str();
    }

    int32_t RelationSettings::NeutralRelationThreshold() const
    {
        return m_neutralRelationThreshold;
    }

    int32_t RelationSettings::FriendRelationThreshold() const
    {
        return m_friendRelationThreshold;
    }

    int32_t RelationSettings::DefaultEnemyGoodwillAmount() const
    {
        return m_defaultEnemyGoodwill;
    }

    int32_t RelationSettings::DefaultNeutralGoodwillAmount() const
    {
        return m_defaultNeutralGoodwill;
    }

    int32_t RelationSettings::DefaultFriendGoodwillAmount() const
    {
        return m_defaultFriendGoodwill;
    }

    int32_t RelationSettings::AttackEnemyGoodwillAmount() const
    {
        return m_attackEnemyGoodwillAmount;
    }

    int32_t RelationSettings::AttackNeutralGoodwillAmount() const
    {
        return m_attackNeutralGoodwillAmount;
    }

    int32_t RelationSettings::AttackFriendGoodwillAmount() const
    {
        return m_attackFriendGoodwillAmount;
    }

    int32_t RelationSettings::AttackEnemyReputationAmount() const
    {
        return m_attackEnemyReputationAmount;
    }

    int32_t RelationSettings::AttackNeutralReputationAmount() const
    {
        return m_attackNeutralReputationAmount;
    }

    int32_t RelationSettings::AttackFriendReputationAmount() const
    {
        return m_attackFriendReputationAmount;
    }

    int32_t RelationSettings::KillEnemyGoodwillAmount() const
    {
        return m_killEnemyGoodwillAmount;
    }

    int32_t RelationSettings::KillNeutralGoodwillAmount() const
    {
        return m_killNeutralGoodwillAmount;
    }

    int32_t RelationSettings::KillFriendGoodwillAmount() const
    {
        return m_killFriendGoodwillAmount;
    }

    int32_t RelationSettings::KillEnemyReputationAmount() const
    {
        return m_killEnemyReputationAmount;
    }

    int32_t RelationSettings::KillNeutralReputationAmount() const
    {
        return m_killNeutralReputationAmount;
    }

    int32_t RelationSettings::KillFriendReputationAmount() const
    {
        return m_killFriendReputationAmount;
    }

    int32_t RelationSettings::KillCommunityMemberGoodwillAmount() const
    {
        return m_killCommunityGoodwillAmount;
    }

    u32 RelationSettings::MinAttackDeltaTime() const
    {
        return m_minAttackDeltaTime;
    }

    u32 RelationSettings::MaxAttackManagerRememberTime() const
    {
        return m_attackMgrRememberTime;
    }

    int32_t RelationSettings::HelpEnemyGoodwillAmount() const
    {
        return m_helpEnemyGoodwillAmount;
    }

    int32_t RelationSettings::HelpNeutralGoodwillAmount() const
    {
        return m_helpNeutralGoodwillAmount;
    }

    int32_t RelationSettings::HelpFriendGoodwillAmount() const
    {
        return m_helpFriendGoodwillAmount;
    }

    int32_t RelationSettings::HelpEnemyReputationAmount() const
    {
        return m_helpEnemyReputationAmount;
    }

    int32_t RelationSettings::HelpNeutralReputationAmount() const
    {
        return m_helpNeutralReputationAmount;
    }

    int32_t RelationSettings::HelpFriendReputationAmount() const
    {
        return m_helpFriendReputationAmount;
    }

    Ivector2 RelationSettings::PersonalGoodwillLimits() const
    {
        return m_personalGoodwillLimits;
    }

    Ivector2 RelationSettings::CommunityGoodwillLimits() const
    {
        return m_communityGoodwillLimits;
    }

    Ivector2 RelationSettings::ReputationLimits() const
    {
        return m_reputationLimits;
    }

    Ivector2 RelationSettings::RankLimits() const
    {
        return m_rankLimits;
    }
}