#ifndef H_KAO2AR_TYPEINFO
#define H_KAO2AR_TYPEINFO

#include <kao2ar/kao2ar.h>

namespace ZookieWizard
{
    class eObject;

    ////////////////////////////////////////////////////////////////
    // KAO2 TYPEINFO CONTAINER
    ////////////////////////////////////////////////////////////////

    struct TypeInfo
    {
        /*[0x00]*/ int32_t id;
        /*[0x04]*/ const char* name;
        /*[0x08]*/ TypeInfo* parent;
        /*[0x10]*/ eObject* (*create)();

        TypeInfo(int _id, const char* _name, TypeInfo* _parent, eObject* (*_create)());

        bool checkHierarchy(const TypeInfo* assumed_type) const;
    };


    ////////////////////////////////////////////////////////////////
    // KAO2 INTERFACES LIST
    ////////////////////////////////////////////////////////////////

    static const int MAX_INTERFACES = 288;

    struct Kao2Interfaces
    {
        /*** Properties ***/

        private:

            int interfacesCount;
            TypeInfo* list[MAX_INTERFACES];

        /*** Methods ***/

        private:

            bool registerTypeInfo(const TypeInfo* info);

        public:

            bool registerInterfaces();

            TypeInfo* getTypeInfo(int id) const;
            TypeInfo* getTypeInfo(const char* name) const;
    };

    extern Kao2Interfaces InterfaceManager;

}

#endif
