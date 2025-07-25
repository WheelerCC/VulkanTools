/*
 * Copyright (c) 2020-2025 Valve Corporation
 * Copyright (c) 2020-2025 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Authors:
 * - Christophe Riccio <christophe@lunarg.com>
 */

#include "style.h"

#include "../vkconfig_core/configurator.h"
#include "../vkconfig_core/type_platform.h"

#include <QSysInfo>
#include <QStyleHints>
#include <QGuiApplication>

QIcon Get(ThemeMode mode, Icon icon) {
    static const char* ICONS[] = {
        "clear.png",              // ICON_CLEAR
        "drag.png",               // ICON_DRAG
        "next.png",               // ICON_NEXT
        "prev.png",               // ICON_PREV
        "down.png",               // ICON_MODE
        "exit.png",               // ICON_EXIT
        "show.png",               // ICON_SHOW
        "hide.png",               // ICON_HIDE
        "search_case.png",        // ICON_SEARCH_CASE
        "search_whole.png",       // ICON_SEARCH_WHOLE
        "search_regex.png",       // ICON_SEARCH_REGEX
        "file_append.png",        // ICON_FILE_APPEND,
        "file_export.png",        // ICON_FILE_EXPORT,
        "file_remove.png",        // ICON_FILE_REMOVE,
        "file_search.png",        // ICON_FILE_SEARCH,
        "folder_append.png",      // ICON_FOLDER_APPEND,
        "folder_export.png",      // ICON_FOLDER_EXPORT,
        "folder_reload.png",      // ICON_FOLDER_RELOAD,
        "folder_remove.png",      // ICON_FOLDER_REMOVE,
        "folder_search.png",      // ICON_FOLDER_SEARCH,
        "options_copy.png",       // ICON_OPTIONS_COPY,
        "options_remove.png",     // ICON_OPTIONS_REMOVE,
        "reload.png",             // ICON_RELOAD,
        "reset.png",              // ICON_RESET,
        "settings_advanced.png",  // ICON_ADVANCED,
        "settings_basic.png",     // ICON_BASIC,
        "system-invalid.png",     // ICON_SYSTEM_INVALID,
        "system-off.png",         // ICON_SYSTEM_OFF,
        "system-on.png",          // ICON_SYSTEM_ON,
    };
    static_assert(std::size(ICONS) == ICON_COUNT);

    if (mode == THEME_MODE_AUTO) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
        if (QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark) {
            mode = THEME_MODE_FORCE_DARK;
        } else
#endif  // QT_VERSION
        {
            mode = THEME_MODE_FORCE_LIGHT;
        }
    }

    std::string path;
    switch (mode) {
        default:
            assert(0);
            break;
        case THEME_MODE_FORCE_LIGHT:
            path = ":/resourcefiles/light/";
            break;
        case THEME_MODE_FORCE_DARK:
            path = ":/resourcefiles/dark/";
            break;
    }

    assert(!path.empty());

    return QIcon((path + ICONS[icon]).c_str());
}
