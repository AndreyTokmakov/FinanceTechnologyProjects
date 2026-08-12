/**============================================================================
Name        : Collections.hpp
Created on  : 23.11.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Collections.hpp
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_COLLECTIONS_HPP
#define FINANCETECHNOLOGYPROJECTS_COLLECTIONS_HPP

namespace collections
{
    void MapWithConstantSize();
    void StaticSortedArray();
    void StaticSortedFlatMap();
    void StaticSortedFlatMap_WithDeletion();

    namespace price_level_storage { void TestAll(); };

}

#endif //FINANCETECHNOLOGYPROJECTS_COLLECTIONS_HPP