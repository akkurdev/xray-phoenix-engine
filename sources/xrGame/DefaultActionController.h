#pragma once
#include <thread>
#include <vector>
#include "CharacterMetricTable.h"
#include "IRelationTypeChanger.h"
#include "RelationSettings.h"
#include "IActionController.h"

class CRelationRegistryWrapper;

namespace Stalker::Mechanics::Relations
{
    /// <summary>
    /// Контроллер действий по умолчанию
    /// </summary>
    class DefaultActionController final : public IActionController
    {
    public:
        DefaultActionController(CRelationRegistryWrapper* wrapper, RelationSettings* settings);
        ~DefaultActionController();

        /// <summary>
        /// Регистрация нового действия актора
        /// </summary>
        /// <param name="attacker">Указатель на атакующую игровую сущность</param>
        /// <param name="defender">Указатель на обороняющуюся игровую сущность</param>
        /// <param name="actionType">Тип действия</param>
        /// <param name="bone">Поврежденная кость скелета персонажа</param>
        virtual void RegisterAction(CEntityAlive* attacker, CEntityAlive* defender, const ActionType& actionType, const u16& bone);

        /// <summary>
        /// Подписка нового наблюдателя
        /// </summary>
        /// <param name="observer">Новый наблюдатель</param>
        virtual void AddObserver(IActionObserver* observer);

        /// <summary>
        /// Отписка существующего наблюдателя
        /// </summary>
        /// <param name="observer">Существующий наблюдатель</param>
        virtual void RemoveObserver(IActionObserver* observer);

        /// <summary>
        /// Оповещение наблюдателей о новом действии актора
        /// </summary>
        /// <param name="action">Новое действие</param>
        virtual void NotifyObservers(const Action& action);

        /// <summary>
        /// Получение списка зарегистрированных ранее действий актора. 
        /// В целях оптимизации, контроллер сохраняет только последнее действие в адрес конкретного NPC.
        /// </summary>
        std::vector<Action> Actions() const;

        /// <summary>
        /// Загрузка списка действий из файла
        /// </summary>
        void Load(IReader& reader);

        /// <summary>
        /// Сохранение списка действий в файл
        /// </summary>
        void Save(IWriter& writer);

    private:
        Action* findAction(const u16& victimId);
        int32_t findActionIndex(const u16& victimId) const;

        bool isTeammate(CInventoryOwner* attacker, CInventoryOwner* defender, bool isFriendlyFireEnabled) const;
        CharacterGoodwill calculateGoodwill(ActionType actionType, RelationType relationType, bool isTeammate) const;
        CharacterReputation calculateReputation(ActionType actionType, RelationType relationType, bool isTeammate) const;
        CharacterRank calculateRank(CInventoryOwner* npc, ActionType actionType, bool isTeammate) const;

        Action buildAction(CEntityAlive* attacker, CEntityAlive* defender, const ActionType& actionType, const u16& bone);
        void addAction(const Action& action);
        void removeAction(const Action& action);

    private:
        RelationSettings* m_settings;
        CharacterMetricTable* m_metricTable;
        IRelationTypeChanger* m_relationChanger;
        std::vector<IActionObserver*> m_observers;
        std::vector<Action> m_actionList;
        std::mutex m_lock;
    };
}