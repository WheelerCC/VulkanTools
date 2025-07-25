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

#include "setting_bool.h"
#include "setting_filesystem.h"
#include "setting_flags.h"
#include "setting_float.h"
#include "setting_frames.h"
#include "setting_group.h"
#include "setting_int.h"
#include "setting_list.h"
#include "setting_string.h"
#include "util.h"
#include "version.h"
#include "layer.h"

#include <cassert>

#include <QJsonArray>
#include <QCheckBox>

SettingType GetSettingType(const char* token) {
    assert(token != nullptr);
    assert(std::strcmp(token, "") != 0);

    for (int i = SETTING_FIRST; i <= SETTING_LAST; ++i) {
        const SettingType type = static_cast<SettingType>(i);
        if (ToUpperCase(token) == GetToken(type)) return type;
    }

    return static_cast<SettingType>(-1);
}

const char* GetToken(SettingType type) {
    assert(type >= SETTING_FIRST && type <= SETTING_LAST);

    static const char* table[] = {
        "STRING",        // SETTING_STRING
        "INT",           // SETTING_INT
        "FLOAT",         // SETTING_FLOAT
        "GROUP",         // SETTING_GROUP
        "SAVE_FILE",     // SETTING_SAVE_FILE
        "LOAD_FILE",     // SETTING_LOAD_FILE
        "SAVE_FOLDER",   // SETTING_SAVE_FOLDER
        "LOAD_FOLDER",   // SETTING_LOAD_FOLDER
        "BOOL",          // SETTING_BOOL
        "BOOL_NUMERIC",  // SETTING_BOOL_NUMERIC_DEPRECATED
        "ENUM",          // SETTING_ENUM
        "FLAGS",         // SETTING_FLAGS
        "FRAMES",        // SETTING_FRAMES
        "LIST"           // SETTING_LIST
    };
    static_assert(std::size(table) == SETTING_COUNT, "The tranlation table size doesn't match the enum number of elements");

    return table[type];
}

const char* GetLayerSettingTypeString(SettingType type) {
    assert(type >= SETTING_FIRST && type <= SETTING_LAST);

    static const char* table[] = {
        "VK_LAYER_SETTING_TYPE_STRING_EXT",   // SETTING_STRING
        "VK_LAYER_SETTING_TYPE_INT32_EXT",    // SETTING_INT
        "VK_LAYER_SETTING_TYPE_FLOAT32_EXT",  // SETTING_FLOAT
        "N/A",                                // SETTING_GROUP
        "VK_LAYER_SETTING_TYPE_STRING_EXT",   // SETTING_SAVE_FILE
        "VK_LAYER_SETTING_TYPE_STRING_EXT",   // SETTING_LOAD_FILE
        "VK_LAYER_SETTING_TYPE_STRING_EXT",   // SETTING_SAVE_FOLDER
        "VK_LAYER_SETTING_TYPE_STRING_EXT",   // SETTING_LOAD_FOLDER
        "VK_LAYER_SETTING_TYPE_BOOL32_EXT",   // SETTING_BOOL
        "VK_LAYER_SETTING_TYPE_INT32_EXT",    // SETTING_BOOL_NUMERIC_DEPRECATED
        "VK_LAYER_SETTING_TYPE_STRING_EXT",   // SETTING_ENUM
        "VK_LAYER_SETTING_TYPE_STRING_EXT",   // SETTING_FLAGS
        "VK_LAYER_SETTING_TYPE_UINT32_EXT",   // SETTING_FRAMES
        "VK_LAYER_SETTING_TYPE_STRING_EXT"    // SETTING_LIST
    };
    static_assert(std::size(table) == SETTING_COUNT, "The tranlation table size doesn't match the enum number of elements");

    return table[type];
}

const char* GetCodeTypeString(SettingType type) {
    assert(type >= SETTING_FIRST && type <= SETTING_LAST);

    static const char* table[] = {
        "const char*",  // SETTING_STRING
        "int32_t",      // SETTING_INT
        "float",        // SETTING_FLOAT
        "N/A",          // SETTING_GROUP
        "const char*",  // SETTING_SAVE_FILE
        "const char*",  // SETTING_LOAD_FILE
        "const char*",  // SETTING_SAVE_FOLDER
        "const char*",  // SETTING_LOAD_FOLDER
        "VkBool32",     // SETTING_BOOL
        "int32_t",      // SETTING_BOOL_NUMERIC_DEPRECATED
        "const char*",  // SETTING_ENUM
        "const char*",  // SETTING_FLAGS
        "uint32_t",     // SETTING_FRAMES
        "const char*"   // SETTING_LIST
    };
    static_assert(std::size(table) == SETTING_COUNT, "The tranlation table size doesn't match the enum number of elements");

    return table[type];
}

bool IsSettingTypeString(SettingType type) {
    return type == SETTING_STRING || type == SETTING_SAVE_FILE || type == SETTING_LOAD_FILE || type == SETTING_SAVE_FOLDER ||
           type == SETTING_LOAD_FOLDER || type == SETTING_ENUM || type == SETTING_FLAGS || type == SETTING_LIST;
}

SettingData::SettingData(const std::string& key, const SettingType& type) : key(key), type(type) { assert(!this->key.empty()); }

bool SettingData::Equal(const SettingData& other) const {
    if (this->key != other.key)
        return false;
    else if (this->type != other.type)
        return false;
    return true;
}

SettingMeta::SettingMeta(Layer& layer, const std::string& key, const SettingType type)
    : key(key), type(type), env(), dependence_mode(DEPENDENCE_NONE), layer(layer) {
    assert(!this->key.empty());
    assert(type >= SETTING_FIRST && type <= SETTING_LAST);
}

SettingMeta::~SettingMeta() {
    for (std::size_t i = 0, n = this->instances.size(); i < n; ++i) {
        delete this->instances[i];
        this->instances[i] = nullptr;
    }
}

bool IsSupported(const SettingMeta* meta) {
    if (meta == nullptr) return false;

    if (meta->view == SETTING_VIEW_HIDDEN) return false;

    if (!IsPlatformSupported(meta->platform_flags)) return false;

    return true;
}

bool SettingMeta::Equal(const SettingMeta& other) const {
    if (this->key != other.key)
        return false;
    else if (this->type != other.type)
        return false;
    else if (this->view != other.view)
        return false;
    else if (this->env != other.env)
        return false;
    else if (this->label != other.label)
        return false;
    else if (this->description != other.description)
        return false;
    else if (this->url != other.url)
        return false;
    else if (this->status != other.status)
        return false;
    else if (this->platform_flags != other.platform_flags)
        return false;
    else if (this->children.size() != other.children.size())
        return false;
    else if (this->dependence.size() != other.dependence.size())
        return false;
    else if (this->dependence_mode != other.dependence_mode)
        return false;
    else {
        for (std::size_t i = 0, n = this->children.size(); i < n; ++i) {
            if (this->children[i] != other.children[i]) {
                return false;
            }
        }

        for (std::size_t i = 0, n = this->dependence.size(); i < n; ++i) {
            if (this->dependence[i] != other.dependence[i]) {
                return false;
            }
        }
    }
    return true;
}

bool operator==(const SettingEnumValue& a, const SettingEnumValue& b) {
    if (a.key != b.key) return false;
    if (a.label != b.label) return false;
    if (a.description != b.description) return false;
    if (a.url != b.url) return false;
    if (a.status != b.status) return false;
    if (a.view != b.view) return false;
    if (a.platform_flags != b.platform_flags) return false;
    return true;
}

bool IsSupported(const SettingEnumValue* value) {
    if (value == nullptr) return false;

    if (value->view == SETTING_VIEW_HIDDEN) return false;

    if (!IsPlatformSupported(value->platform_flags)) return false;

    return true;
}

SettingMeta* FindSetting(SettingMetaSet& settings, const char* key) {
    for (std::size_t i = 0, n = settings.size(); i < n; ++i) {
        if (settings[i]->key == key) {
            return settings[i];
        }

        SettingMeta* child = FindSetting(settings[i]->children, key);
        if (child != nullptr) {
            return child;
        }

        if (IsEnum(settings[i]->type) || IsFlags(settings[i]->type)) {
            SettingMetaEnum& setting_meta_enum = static_cast<SettingMetaEnum&>(*settings[i]);

            for (std::size_t j = 0, o = setting_meta_enum.enum_values.size(); j < o; ++j) {
                SettingMeta* setting_meta = FindSetting(setting_meta_enum.enum_values[j].settings, key);
                if (setting_meta != nullptr) {
                    return setting_meta;
                }
            }
        }
    }

    return nullptr;
}

const SettingMeta* FindSetting(const SettingMetaSet& settings, const char* key) {
    for (std::size_t i = 0, n = settings.size(); i < n; ++i) {
        if (settings[i]->key == key) return settings[i];

        const SettingMeta* child = FindSetting(settings[i]->children, key);
        if (child != nullptr) {
            return child;
        }

        if (IsEnum(settings[i]->type) || IsFlags(settings[i]->type)) {
            const SettingMetaEnum& setting_meta_enum = static_cast<const SettingMetaEnum&>(*settings[i]);

            for (std::size_t j = 0, o = setting_meta_enum.enum_values.size(); j < o; ++j) {
                const SettingMeta* setting_meta = FindSetting(setting_meta_enum.enum_values[j].settings, key);
                if (setting_meta != nullptr) {
                    return setting_meta;
                }
            }
        }
    }

    return nullptr;
}

SettingData* FindSetting(SettingDataSet& settings, const char* key) {
    for (std::size_t i = 0, n = settings.size(); i < n; ++i) {
        if (settings[i]->key == key) {
            return settings[i];
        }
    }

    return nullptr;
}

const SettingData* FindSetting(SettingDataSetConst& settings, const char* key) {
    for (std::size_t i = 0, n = settings.size(); i < n; ++i) {
        if (settings[i]->key == key) {
            return settings[i];
        }
    }

    return nullptr;
}

const SettingData* FindSetting(const SettingDataSet& settings, const char* key) {
    for (std::size_t i = 0, n = settings.size(); i < n; ++i) {
        if (settings[i]->key == key) {
            return settings[i];
        }
    }

    return nullptr;
}

static std::size_t CountStandardSettings(const SettingMetaSet& settings) {
    std::size_t count = 0;

    for (std::size_t i = 0, n = settings.size(); i < n; ++i) {
        if (settings[i]->view == SETTING_VIEW_STANDARD) {
            ++count;
        }
    }

    return count;
}

std::size_t CountSettings(const SettingMetaSet& settings, bool only_standard) {
    std::size_t count = only_standard ? ::CountStandardSettings(settings) : settings.size();

    for (std::size_t i = 0, n = settings.size(); i < n; ++i) {
        count += CountSettings(settings[i]->children);

        if (IsEnum(settings[i]->type) || IsFlags(settings[i]->type)) {
            const SettingMetaEnumeration& meta_enum = static_cast<const SettingMetaEnumeration&>(*settings[i]);

            for (std::size_t j = 0, o = meta_enum.enum_values.size(); j < o; ++j) {
                count += CountSettings(meta_enum.enum_values[j].settings);
            }
        }
    }

    return count;
}

bool CheckSettingOverridden(const SettingMeta& meta) {
    // If the setting environment variable is set, then the setting is overridden and can't be modified
    if (!meta.env.empty()) {
        if (!qgetenv(meta.env.c_str()).isEmpty()) {
            return true;
        }
    }

    return false;
}

std::string GetSettingOverride(const SettingMeta& meta) {
    std::string result;

    // If the setting environment variable is set, then the setting is overridden and can't be modified
    if (!meta.env.empty()) {
        result = qgetenv(meta.env.c_str()).toStdString();
    }

    return result;
}

SettingDependenceMode CheckDependence(const SettingMeta& meta, const SettingDataSet& data_set) {
    switch (meta.dependence_mode) {
        default:
        case DEPENDENCE_NONE: {
            return SETTING_DEPENDENCE_ENABLE;
        }
        case DEPENDENCE_ALL: {
            for (std::size_t i = 0, n = meta.dependence.size(); i < n; ++i) {
                const SettingDependenceMode mode =
                    meta.dependence[i]->type == SETTING_ENUM ? SETTING_DEPENDENCE_HIDE : SETTING_DEPENDENCE_DISABLE;
                const SettingData* data = FindSetting(data_set, meta.dependence[i]->key.c_str());
                if (data == nullptr) {
                    return mode;
                }

                if (*data != *meta.dependence[i]) {
                    return mode;
                }
            }
            return SETTING_DEPENDENCE_ENABLE;
        }
        case DEPENDENCE_ANY: {
            for (std::size_t i = 0, n = meta.dependence.size(); i < n; ++i) {
                const SettingData* data = FindSetting(data_set, meta.dependence[i]->key.c_str());
                if (data == nullptr) continue;

                if (meta.dependence[i]->type == SETTING_FLAGS) {
                    const SettingDataFlags& data_flags = static_cast<const SettingDataFlags&>(*data);
                    const SettingDataFlags& dep_flags = static_cast<const SettingDataFlags&>(*meta.dependence[i]);

                    std::size_t found_flags = 0;
                    for (std::size_t j = 0, o = dep_flags.value.size(); j < o; ++j) {
                        if (IsStringFound(data_flags.value, dep_flags.value[j])) {
                            ++found_flags;
                        }
                    }
                    if (found_flags == dep_flags.value.size()) return SETTING_DEPENDENCE_ENABLE;
                }

                if (*data == *meta.dependence[i]) {
                    return SETTING_DEPENDENCE_ENABLE;
                }
            }
            return SETTING_DEPENDENCE_DISABLE;
        }
    }
}

const char* GetToken(DependenceMode type) {
    static const char* table[] = {
        "NONE",  // DEPENDENCE_NONE
        "ALL",   // DEPENDENCE_ALL
        "ANY"    // DEPENDENCE_ANY
    };
    static_assert(std::size(table) == DEPENDENCE_COUNT, "The tranlation table size doesn't match the enum number of elements");

    return table[type];
}

DependenceMode GetDependenceMode(const char* token) {
    for (std::size_t i = 0, n = DEPENDENCE_COUNT; i < n; ++i) {
        const DependenceMode value = static_cast<DependenceMode>(i);
        if (std::strcmp(GetToken(value), token) == 0) return value;
    }

    assert(0);
    return static_cast<DependenceMode>(-1);
}

void CheckMessage(IgnoredMessages& ignored_messages, const SettingMeta& meta, SettingDataSet& data_set) {
    for (std::size_t i = 0, n = meta.messages.size(); i < n; ++i) {
        const Message& message = meta.messages[i];

        if (!message.Triggered(data_set)) {
            continue;
        }

        bool show_message_box = false;
        auto it = ignored_messages.find(message.key);
        if (it != ignored_messages.end()) {
            if (it->second < message.version) {
                show_message_box = true;
            }
        } else {
            show_message_box = true;
        }

        QMessageBox::StandardButton button = ::GetStandardButton(message.actions[message.default_action].type);
        if (show_message_box) {
            QMessageBox alert;
            alert.setWindowTitle(message.title.c_str());
            alert.setText(message.description.c_str());
            if (!message.informative.empty()) {
                alert.setInformativeText(message.informative.c_str());
            }
            alert.setIcon(::GetIcon(message.severity));
            alert.setCheckBox(new QCheckBox("Do not show again."));

            alert.setStandardButtons(message.GetStandardButtons());
            alert.setDefaultButton(button);
            button = static_cast<QMessageBox::StandardButton>(alert.exec());

            if (alert.checkBox()->isChecked()) {
                if (it != ignored_messages.end()) {
                    ignored_messages[message.key] = message.version;
                } else {
                    ignored_messages.insert(std::make_pair(message.key, message.version));
                }
            }
        }

        message.Apply(data_set, button);

        break;
    }
}
