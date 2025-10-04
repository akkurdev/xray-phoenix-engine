#pragma once
#include <xrCore/xrCore.h>
#include <script_export_space.h>
#include "TurretTargetType.h"

class CCar;
class CInventoryOwner;

/// <summary>
/// ������������� �������������� ����
/// </summary>
constexpr uint16_t UNKNOWN_TARGET_ID = (uint16_t)(-1);

/// <summary>
/// ����� ������������ �������
/// </summary>
class TurretGunner final
{
public:
    /// <summary>
    /// ������������� ����� ��������� <see cref="TurretGunner"/>
    /// </summary>
    TurretGunner();

    /// <summary>
    /// ���������� ������� ��������� <see cref="TurretGunner"/>
    /// </summary>
    ~TurretGunner() = default;

    /// <summary>
    /// ������� �� ����������� �������
    /// </summary>
    bool IsActive() const;

    /// <summary>
    /// ������ ������� ����. 
    /// </summary>
    CScriptGameObject* CurrentTarget() const;

    /// <summary>
    /// ����������� ������������ �������
    /// </summary>
    std::string Community() const;

    /// <summary>
    /// ��������� �������� ������ �����
    /// </summary>
    uint32_t SearchInterval() const;

    /// <summary>
    /// ��������� ������ �����
    /// </summary>
    float SearchDistance() const;

    /// <summary>
    /// ��� �����
    /// </summary>
    TurretTargetType TargetType() const;

    /// <summary>
    /// ����� �� ������������� �� ����� ����, ���� ������� ������ ���������� ��� ���������
    /// </summary>
    bool IsSwitchTargetEnabled() const;

    /// <summary>
    /// ������ ������ ����
    /// </summary>
    float BlindSpotRadius() const;

    /// <summary>
    /// ����������/������������ ������������ �������
    /// </summary>
    /// <param name="activateCondition"> - ������� ���������/�����������</param>
    void Activate(bool activateCondition);

    /// <summary>
    /// ������������� �������������� ������������ ������� � ��������� �����������
    /// </summary>
    /// <param name="community"> - ����������� ������������ �������</param>
    void SetCommunity(const std::string& community);

    /// <summary>
    /// ������������� �������� ������ ����� �����
    /// </summary>
    /// <param name="interval"> - �������� ������ �����</param>
    void SetSearchInterval(uint32_t interval);

    /// <summary>
    /// ������������� ��������� ������ �����
    /// </summary>
    /// <param name="distance"> - ��������� ������ �����</param>
    void SetSearchDistance(float distance);

    /// <summary>
    /// ������������� ��� �����
    /// </summary>
    /// <param name="type"> - ��� �����</param>
    void SetTargetType(TurretTargetType type);

    /// <summary>
    /// ������������� ����������� ������������� ����� ������
    /// </summary>
    /// <param name="isSwitchEnabled"> - ����� �� ������������� �� ������ ����</param>
    void SetTargetSwitch(bool isSwitchEnabled);

    /// <summary>
    /// ������������� ������ ������ ���� �����
    /// </summary>
    /// <param name="blindSpotRadius"> - ������ ������ ���� �����</param>
    virtual void SetBlindSpotRadius(float blindSpotRadius);

    /// <summary>
    /// ��������� ��������� ������������ �������
    /// </summary>
    /// <param name="selfPosition"> - ����������� ������� ������� � ������������</param>
    void Update(CCar* car);

    /// <summary>
    /// ������������ ��������� � ���������� ��������, 
    /// ��� ���������� �����
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
