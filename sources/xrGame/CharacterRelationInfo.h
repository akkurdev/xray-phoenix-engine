#pragma once
#include "object_interfaces.h"
#include "RelationDefinitions.h"

namespace Stalker::Mechanics::Relations
{
    /// <summary>
    /// Информация об отношениях конкретного персонажа
    /// </summary>
    struct CharacterRelationInfo final : public IPureSerializeObject<IReader, IWriter>
    {
    public:
        CharacterRelationInfo();
        ~CharacterRelationInfo() = default;

        /// <summary>
        /// Очистка всей информации об отношениях
        /// </summary>
        virtual void clear();

        /// <summary>
        /// Загрузка информации об отношениях
        /// </summary>
        virtual void load(IReader& reader);

        /// <summary>
        /// Сохранение информации об отношениях
        /// </summary>
        virtual void save(IWriter& writer);

        /// <summary>
        /// Персональные отношения персонажа с другими обитателями Зоны
        /// </summary>
        PersonalRelationMap PersonalRelations;

        /// <summary>
        /// Отношения персонажа с группировками
        /// </summary>
        CommunityRelationMap CommunityRelations;

        /// <summary>
        /// Перечень недоброжелателей, когда-либо проявлявших агрессию
        /// </summary>
        xr_vector<u16> Foes;
    };
}