#pragma once
#include <vector>
#include "IActionObservable.h"
#include "entity_alive.h"

namespace Stalker::Mechanics::Relations
{
    /// <summary>
    /// Интерфейс контроллера действий персонажей
    /// </summary>
    __interface IActionController : public IActionObservable
    {
        /// <summary>
        /// Регистрация нового действия актора
        /// </summary>
        /// <param name="attacker">Указатель на атакующую игровую сущность</param>
        /// <param name="defender">Указатель на обороняющуюся игровую сущность</param>
        /// <param name="actionType">Тип действия</param>
        /// <param name="bone">Поврежденная кость скелета персонажа</param>
        void RegisterAction(CEntityAlive* attacker, CEntityAlive* defender, const ActionType& actionType, const u16& bone);

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
    };
}