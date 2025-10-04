#pragma once
#include <xrCore/xrCore.h>
#include <string_view>

namespace Stalker::Mechanics::Relations
{
    /// <summary>
    /// Настройки отношений между персонажами
    /// </summary>
    class RelationSettings final
    {
    public:
        RelationSettings();
        ~RelationSettings() = default;

        /// <summary>
        /// Включен ли скриптовый обработчик действий
        /// </summary>
        bool IsScriptActionHandlerEnabled() const;

        /// <summary>
        /// Разрешен ли огонь NPC по своим (только для движкового обработчика)
        /// </summary>
        bool IsFriendlyFireEnabled() const;

        /// <summary>
        /// Имя скриптовой функции-обработчика действий
        /// </summary>
        std::string_view ScriptActionHandlerName() const;

        /// <summary>
        /// Минимальное пороговое значение нейтрального отношения
        /// </summary>
        int32_t NeutralRelationThreshold() const;

        /// <summary>
        /// Минимальное пороговое значение дружественного отношения
        /// </summary>
        int32_t FriendRelationThreshold() const;

        /// <summary>
        /// Благосклонность враждебных NPC по умолчанию
        /// </summary>
        int32_t DefaultEnemyGoodwillAmount() const;

        /// <summary>
        /// Благосклонность нейтральных NPC по умолчанию
        /// </summary>
        int32_t DefaultNeutralGoodwillAmount() const;

        /// <summary>
        /// Благосклонность дружественных NPC по умолчанию
        /// </summary>
        int32_t DefaultFriendGoodwillAmount() const;

        /// <summary>
        /// Очки благосклонности, начисляемые за агрессию в сторону враждебного NPC в спокойной обстановке
        /// </summary>
        int32_t AttackEnemyGoodwillAmount() const;

        /// <summary>
        /// Очки благосклонности, начисляемые за агрессию в сторону нейтрального NPC в спокойной обстановке
        /// </summary>
        int32_t AttackNeutralGoodwillAmount() const;

        /// <summary>
        /// Очки благосклонности, начисляемые за агрессию в сторону дружественного NPC в спокойной обстановке
        /// </summary>
        int32_t AttackFriendGoodwillAmount() const;

        /// <summary>
        /// Очки репутации, начисляемые за агрессию в сторону враждебного NPC в спокойной обстановке
        /// </summary>
        int32_t AttackEnemyReputationAmount() const;

        /// <summary>
        /// Очки репутации, начисляемые за агрессию в сторону нейтрального NPC в спокойной обстановке
        /// </summary>
        int32_t AttackNeutralReputationAmount() const;

        /// <summary>
        /// Очки репутации, начисляемые за агрессию в сторону дружественного NPC в спокойной обстановке
        /// </summary>
        int32_t AttackFriendReputationAmount() const;

        /// <summary>
        /// Очки благосклонности, начисляемые за убийство враждебного NPC
        /// </summary>
        int32_t KillEnemyGoodwillAmount() const;

        /// <summary>
        /// Очки благосклонности, начисляемые за убийство нейтрального NPC
        /// </summary>
        int32_t KillNeutralGoodwillAmount() const;

        /// <summary>
        /// Очки благосклонности, начисляемые за убийство дружественного NPC
        /// </summary>
        int32_t KillFriendGoodwillAmount() const;

        /// <summary>
        /// Очки репутации, начисляемые за убийство враждебного NPC
        /// </summary>
        int32_t KillEnemyReputationAmount() const;

        /// <summary>
        /// Очки репутации, начисляемые за убийство нейтрального NPC
        /// </summary>
        int32_t KillNeutralReputationAmount() const;

        /// <summary>
        /// Очки репутации, начисляемые за убийство дружественного NPC
        /// </summary>
        int32_t KillFriendReputationAmount() const;

        /// <summary>
        /// Очки благосклонности, начисляемые за убийство члена группировки
        /// </summary>
        int32_t KillCommunityMemberGoodwillAmount() const;

        /// <summary>
        /// Минимальное время, через которое снова будет зарегистрирована атака на персонажа, 
        /// и вычтется attack_goodwill и attack_reputation. 
        /// (работает аналогично и при помощи другим персонажам в бою)
        /// </summary>
        u32 MinAttackDeltaTime() const;

        /// <summary>
        /// Максимальное время, в течение которого менеджер атак помнит про поединок
        /// </summary>
        u32 MaxAttackManagerRememberTime() const;

        /// <summary>
        /// Очки благосклонности, начисляемые за помощь враждебному NPC
        /// </summary>
        int32_t HelpEnemyGoodwillAmount() const;

        /// <summary>
        /// Очки благосклонности, начисляемые за помощь нейтральному NPC
        /// </summary>
        int32_t HelpNeutralGoodwillAmount() const;

        /// <summary>
        /// Очки благосклонности, начисляемые за помощь дружественному NPC
        /// </summary>
        int32_t HelpFriendGoodwillAmount() const;

        /// <summary>
        /// Очки репутации, начисляемые за помощь враждебному NPC
        /// </summary>
        int32_t HelpEnemyReputationAmount() const;

        /// <summary>
        /// Очки репутации, начисляемые за помощь нейтральному NPC
        /// </summary>
        int32_t HelpNeutralReputationAmount() const;

        /// <summary>
        /// Очки репутации, начисляемые за помощь дружественному NPC
        /// </summary>
        int32_t HelpFriendReputationAmount() const;

        /// <summary>
        /// Диапазон значений персональной благосклонности
        /// </summary>
        Ivector2 PersonalGoodwillLimits() const;

        /// <summary>
        /// Диапазон значений благосклонности группировки
        /// </summary>
        Ivector2 CommunityGoodwillLimits() const;

        /// <summary>
        /// Диапазон значений репутации
        /// </summary>
        Ivector2 ReputationLimits() const;

        /// <summary>
        /// Диапазон значений ранга
        /// </summary>
        Ivector2 RankLimits() const;

    private:
        std::string m_scriptHandlerName;
        bool    m_isScriptActionHandlerEnabled;
        bool    m_isFriendlyFireEnabled;
        int32_t m_neutralRelationThreshold;
        int32_t m_friendRelationThreshold;
        int32_t m_defaultEnemyGoodwill;
        int32_t m_defaultNeutralGoodwill;
        int32_t m_defaultFriendGoodwill;
        int32_t m_attackEnemyGoodwillAmount;
        int32_t m_attackNeutralGoodwillAmount;
        int32_t m_attackFriendGoodwillAmount;
        int32_t m_attackEnemyReputationAmount;
        int32_t m_attackNeutralReputationAmount;
        int32_t m_attackFriendReputationAmount;
        int32_t m_killEnemyGoodwillAmount;
        int32_t m_killNeutralGoodwillAmount;
        int32_t m_killFriendGoodwillAmount;
        int32_t m_killEnemyReputationAmount;
        int32_t m_killNeutralReputationAmount;
        int32_t m_killFriendReputationAmount;
        u32     m_minAttackDeltaTime;
        u32     m_attackMgrRememberTime;
        int32_t m_helpEnemyGoodwillAmount;
        int32_t m_helpNeutralGoodwillAmount;
        int32_t m_helpFriendGoodwillAmount;
        int32_t m_helpEnemyReputationAmount;
        int32_t m_helpNeutralReputationAmount;
        int32_t m_helpFriendReputationAmount;
        int32_t m_killCommunityGoodwillAmount;
        Ivector2 m_personalGoodwillLimits;
        Ivector2 m_communityGoodwillLimits;
        Ivector2 m_reputationLimits;
        Ivector2 m_rankLimits;
    };
}