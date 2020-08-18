#include <kao2engine/eXRefManager.h>
#include <kao2ar/Archive.h>
#include <kao2ar/eDrawContext.h>

#include <kao2engine/eXRefTarget.h>
#include <kao2engine/eScene.h>

namespace ZookieWizard
{

    ////////////////////////////////////////////////////////////////
    // eXRefManager interface
    // <kao2.004AD7D0> (constructor)
    // <kao2.004AD900> (destructor)
    ////////////////////////////////////////////////////////////////

    TypeInfo E_XREFMANAGER_TYPEINFO
    (
        E_XREFMANAGER_ID,
        "eXRefManager",
        &E_NODE_TYPEINFO,
        []() -> eObject*
        {
            return new eXRefManager;
        }
    );

    TypeInfo* eXRefManager::getType() const
    {
        return &E_XREFMANAGER_TYPEINFO;
    }

    eXRefManager::eXRefManager()
    : eNode()
    {
        /*[0x1C]*/ flags |= 0x70000000;
        /*[0x14]*/ name = "$XRefManager";
    }

    eXRefManager::~eXRefManager() {}


    ////////////////////////////////////////////////////////////////
    // eXRefManager serialization
    // <kao2.004AE730>
    ////////////////////////////////////////////////////////////////
    void eXRefManager::serialize(Archive &ar)
    {
        int32_t i;
        eXRefTarget* test_xref_taget;

        eNode::serialize(ar);

        xrefs.serialize(ar, &E_XREFTARGET_TYPEINFO);

        /********************************/
        /* Saving external models... */

        if (ar.isInExportProxiesMode())
        {
            for (i = 0; i < xrefs.getSize(); i++)
            {
                test_xref_taget = (eXRefTarget*)xrefs.getIthChild(i);

                if (nullptr != test_xref_taget)
                {
                    /* These targets already start with "build/win32" names */

                    test_xref_taget->exportTarget
                    (
                        ar,
                        (AR_MODE_ABSOLUTE_PATH | AR_MODE_XREF_PATH)
                    );
                }
            }
        }
    }


    ////////////////////////////////////////////////////////////////
    // eXRefManager: destroy before dereferencing
    ////////////////////////////////////////////////////////////////
    void eXRefManager::destroyNode()
    {
        int32_t i;
        eXRefTarget* test_xref_taget;
        eNode* nested_scene;

        for (i = 0; i < xrefs.getSize(); i++)
        {
            test_xref_taget = (eXRefTarget*)xrefs.getIthChild(i);

            if (nullptr != test_xref_taget)
            {
                nested_scene = test_xref_taget->getLocalScene();

                if (nullptr != nested_scene)
                {
                    nested_scene->destroyNode();
                }
            }
        }

        eNode::destroyNode();
    }


    ////////////////////////////////////////////////////////////////
    // eXRefManager: render each child eXRefTarget
    ////////////////////////////////////////////////////////////////
    void eXRefManager::renderNode(eDrawContext &draw_context) const
    {
        eXRefTarget* test_xref_taget;

        if (GUI::drawFlags::DRAW_FLAG_PROXIES & draw_context.getDrawFlags())
        {
            for (int32_t i = 0; i < xrefs.getSize(); i++)
            {
                if (nullptr != (test_xref_taget = (eXRefTarget*)xrefs.getIthChild(i)))
                {
                    test_xref_taget->renderXRefScene(draw_context);
                }
            }
        }
    }

}
