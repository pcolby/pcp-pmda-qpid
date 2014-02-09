/*
 * Copyright 2013-2014 Paul Colby
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <qpid/console/ClassKey.h>
#include <qpid/console/Object.h>
#include <qpid/console/Schema.h>

class ConsoleUtils {

public:
    enum ObjectSchemaType {
        Broker,
        Queue,
        System,
        Other
    };

    static std::string getName(const qpid::console::Object &object,
                               const bool allowNodeName = true);

    static ObjectSchemaType getType(const qpid::console::Object &object);

    static ObjectSchemaType getType(const qpid::console::ClassKey &classKey);

    static std::string qmfTypeCodeToString(const uint8_t typeCode);

    static std::string toString(const qpid::console::ClassKey &classKey);

    static std::string toString(const qpid::console::Object &object);

    static std::string toString(const qpid::console::ObjectId &id);

};
