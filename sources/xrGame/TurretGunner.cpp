#include "stdafx.h"
#include "Level.h"
#include "entity_alive.h"
#include "InventoryOwner.h"
#include "TurretGunner.h"
#include "Car.h"
#include "GameObject.h"
#include "RelationSystem.h"
#include "script_game_object.h"
#include <Actor.h>

using namespace luabind;

TurretGunner::TurretGunner()
    :
    m_targets(std::map<uint16_t, CGameObject*> {}),
    m_targetType(TurretTargetType::None),
    m_isActive(false),
    m_community(""),
    m_searchInterval(0u),
    m_searchDistance(0.f),
    m_isSwitchTargetEnabled(false),
    m_blindSpotRadius(0.f),
    m_currentTargetId((uint16_t)(-1)),
    m_lastSearchTime(0u)
{
}

bool TurretGunner::IsActive() const
{
    return m_isActive;
}

CScriptGameObject* TurretGunner::CurrentTarget() const
{
    return m_currentTargetId != UNKNOWN_TARGET_ID && m_targets.contains(m_currentTargetId)
        ? m_targets.at(m_currentTargetId)->lua_game_object()
        : nullptr;
}

std::string TurretGunner::Community() const
{
    return m_community;
}

uint32_t TurretGunner::SearchInterval() const
{
    return m_searchInterval;
}

float TurretGunner::SearchDistance() const
{
    return m_searchDistance;
}

TurretTargetType TurretGunner::TargetType() const
{
    return m_targetType;
}

bool TurretGunner::IsSwitchTargetEnabled() const
{
    return m_isSwitchTargetEnabled;
}

float TurretGunner::BlindSpotRadius() const
{
    return m_blindSpotRadius;
}

void TurretGunner::Activate(bool activateCondition)
{
    m_isActive = activateCondition;
}

void TurretGunner::SetCommunity(const std::string& community)
{
    m_community = community;
}

void TurretGunner::SetSearchInterval(uint32_t interval)
{
    m_searchInterval = interval;
}

void TurretGunner::SetSearchDistance(float distance)
{
    m_searchDistance = distance;
}

void TurretGunner::SetTargetType(TurretTargetType type)
{
    m_targetType = type;
}

void TurretGunner::SetTargetSwitch(bool isSwitchEnabled)
{
    m_isSwitchTargetEnabled = isSwitchEnabled;
}

void TurretGunner::SetBlindSpotRadius(float blindSpotRadius)
{
    m_blindSpotRadius = blindSpotRadius;
}

void TurretGunner::Update(CCar* car)
{
    if (!g_actor || !car || car->IsExploded() || !m_isActive || m_targetType == TurretTargetType::None)
    {
        return;
    }

    if (Device.dwTimeGlobal - m_lastSearchTime >= m_searchInterval)
    {
        SearchTargets(car->Position());
        m_lastSearchTime = Device.dwTimeGlobal;
    }
    
    m_currentTargetId = SelectCurrentTarget(car);
}

void TurretGunner::SearchTargets(const Fvector& selfPosition)
{
    m_targets.clear();
    
    xr_vector<CObject*> nearest{};
    Level().ObjectSpace.GetNearest(nearest, selfPosition, m_searchDistance, NULL);

    for (auto obj : nearest)
    {
        CGameObject* gameObject = smart_cast<CGameObject*>(obj);
        if (!gameObject || m_targets.contains(gameObject->ID()))
        {
            continue;
        }

        auto entity = smart_cast<CEntityAlive*>(gameObject);
        if (!entity || !entity->g_Alive())
        {
            continue;
        }

        if (((m_targetType == TurretTargetType::Human) && !entity->human_being()) ||
            ((m_targetType == TurretTargetType::Mob) && entity->human_being()))
        {
            continue;
        }

        auto invOwner = obj->ID() == 0
            ? g_actor->cast_inventory_owner() 
            : smart_cast<CInventoryOwner*>(gameObject);

        if (!invOwner)
        {
            continue;
        }

        if (entity->human_being())
        {
            CHARACTER_COMMUNITY community;
            community.set(m_community.c_str());

            auto goodwill = RelationSystem::Instance().Executor()->RelationType()->GetGoodwillBetweenCommunities(community.index(), invOwner->Community());
            auto enemyGoodwill = RelationSystem::Instance().Settings().DefaultEnemyGoodwillAmount();

            if (goodwill <= enemyGoodwill)
            {
                m_targets.insert({ obj->ID(), gameObject });
            }            
        }
        else
        {
            m_targets.insert({ obj->ID(), gameObject });
        }        
    }
}

uint16_t TurretGunner::SelectCurrentTarget(CCar* car)
{
    if (car == nullptr || m_targets.empty())
    {
        return UNKNOWN_TARGET_ID;
    }

    std::map<float, uint16_t> distances {};

    for (auto target : m_targets)
    {
        auto targetPos = target.second->Position();
        auto distance = targetPos.distance_to(car->Position()); 

        if (ValidateTarget(car, target.second))
        {
            distances.insert({ distance, target.first });
        }        
    }

    return distances.empty() 
        ? UNKNOWN_TARGET_ID 
        : distances.begin()->second;
}

bool TurretGunner::ValidateTarget(CCar* car, CGameObject* target)
{
    if (!car || !target)
    {
        return false;
    }

    auto targetPos = target->Position();

    auto isAlive = target->cast_entity_alive()->g_Alive();
    auto atBlindSpot = car->Position().distance_to(targetPos) <= m_blindSpotRadius;
    auto isVisible = car->isObjectVisible(target->lua_game_object());

    return isAlive && isVisible && !atBlindSpot;
}

void TurretGunner::script_register(lua_State* L)
{
    module(L)[class_<TurretGunner>("TurretGunner")
        .enum_("target_type")[
            value("TargetNone", int32_t(TurretTargetType::None)), 
            value("TargetHuman", int32_t(TurretTargetType::Human)),
            value("TargetMob", int32_t(TurretTargetType::Mob)),
            value("TargetAll", int32_t(TurretTargetType::All))]
        .def("is_active", &TurretGunner::IsActive)
        .def("current_target", &TurretGunner::CurrentTarget)
        .def("community", &TurretGunner::Community)
        .def("search_interval", &TurretGunner::SearchInterval)
        .def("search_distance", &TurretGunner::SearchDistance)
        .def("blind_spot_radius", &TurretGunner::BlindSpotRadius)
        .def("is_switch_target_enabled", &TurretGunner::IsSwitchTargetEnabled)
        .def("activate", &TurretGunner::Activate)
        .def("set_community", &TurretGunner::SetCommunity)
        .def("set_search_interval", &TurretGunner::SetSearchInterval)
        .def("set_search_distance", &TurretGunner::SetSearchDistance)
        .def("set_target_type", (void(TurretGunner::*)(int32_t)) &TurretGunner::SetTargetType)
        .def("set_target_switch", &TurretGunner::SetTargetSwitch)
        .def("set_blind_spot_radius", &TurretGunner::SetBlindSpotRadius)
        .def(constructor<>())];
}
