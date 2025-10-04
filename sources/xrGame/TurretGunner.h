#pragma once
#include <xrCore/xrCore.h>
#include <script_export_space.h>
#include "TurretTargetType.h"

class CCar;
class CInventoryOwner;

/// <summary>
/// Идентификатор несуществующей цели
/// </summary>
constexpr uint16_t UNKNOWN_TARGET_ID = (uint16_t)(-1);

/// <summary>
/// Класс виртуального стрелка
/// </summary>
class TurretGunner final
{
public:
    /// <summary>
    /// Предоставляет новый экземпляр <see cref="TurretGunner"/>
    /// </summary>
    TurretGunner();

    /// <summary>
    /// Уничтожает текущий экземпляр <see cref="TurretGunner"/>
    /// </summary>
    ~TurretGunner() = default;

    /// <summary>
    /// Активен ли виртуальный стрелок
    /// </summary>
    bool IsActive() const;

    /// <summary>
    /// Объект текущей цели. 
    /// </summary>
    CScriptGameObject* CurrentTarget() const;

    /// <summary>
    /// Группировка виртуального стрелка
    /// </summary>
    std::string Community() const;

    /// <summary>
    /// Временной интервал поиска целей
    /// </summary>
    uint32_t SearchInterval() const;

    /// <summary>
    /// Дистанция поиска целей
    /// </summary>
    float SearchDistance() const;

    /// <summary>
    /// Тип целей
    /// </summary>
    TurretTargetType TargetType() const;

    /// <summary>
    /// Можно ли переключаться на новую цель, если текущая больше недоступна для поражения
    /// </summary>
    bool IsSwitchTargetEnabled() const;

    /// <summary>
    /// Радиус слепой зоны
    /// </summary>
    float BlindSpotRadius() const;

    /// <summary>
    /// Активирует/деактивирует виртуального стрелка
    /// </summary>
    /// <param name="activateCondition"> - Условие активации/деактивации</param>
    void Activate(bool activateCondition);

    /// <summary>
    /// Устанавливает принадлежность виртуального стрелка к указанной группировке
    /// </summary>
    /// <param name="community"> - Группировка виртуального стрелка</param>
    void SetCommunity(const std::string& community);

    /// <summary>
    /// Устанавливает интервал поиска новых целей
    /// </summary>
    /// <param name="interval"> - Интервал поиска целей</param>
    void SetSearchInterval(uint32_t interval);

    /// <summary>
    /// Устанавливает дистанцию поиска целей
    /// </summary>
    /// <param name="distance"> - Дистанция поиска целей</param>
    void SetSearchDistance(float distance);

    /// <summary>
    /// Устанавливает тип целей
    /// </summary>
    /// <param name="type"> - Тип целей</param>
    void SetTargetType(TurretTargetType type);

    /// <summary>
    /// Устанавливает возможность переключаться между целями
    /// </summary>
    /// <param name="isSwitchEnabled"> - Можно ли переключаться на другую цель</param>
    void SetTargetSwitch(bool isSwitchEnabled);

    /// <summary>
    /// Устанавливает радиус слепой зоны башни
    /// </summary>
    /// <param name="blindSpotRadius"> - Радиус слепой зоны башни</param>
    virtual void SetBlindSpotRadius(float blindSpotRadius);

    /// <summary>
    /// Обновляет состояние виртуального стрелка
    /// </summary>
    /// <param name="selfPosition"> - Собственная позиция стрелка в пространстве</param>
    void Update(CCar* car);

    /// <summary>
    /// Экспортирует экземпляр в скриптовую оболочку, 
    /// для управления извне
    /// </summary>
    static void script_register(lua_State* L);

private:
    void SearchTargets(const Fvector& selfPosition);

    uint16_t SelectCurrentTarget(CCar* car);
    bool ValidateTarget(CCar* car, CGameObject* target);

private:
    std::map<uint16_t, CGameObject*> m_targets;
    std::string m_community;

    TurretTargetType m_targetType;
    bool m_isActive;
    uint32_t m_searchInterval;
    float m_searchDistance;    
    bool m_isSwitchTargetEnabled; 
    float m_blindSpotRadius;

    uint16_t m_currentTargetId;
    uint32_t m_lastSearchTime;
};

add_to_type_list(TurretGunner)
#undef script_type_list
#define script_type_list save_type_list(TurretGunner)
