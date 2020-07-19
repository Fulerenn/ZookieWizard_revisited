#include <kao2engine/ePivot.h>
#include <kao2ar/Archive.h>

#include <kao2engine/eTrack.h>

namespace ZookieWizard
{

    ////////////////////////////////////////////////////////////////
    // ePivot interface
    // <kao2.00485CC0> (constructor)
    // <kao2.004860E0> (destructor)
    ////////////////////////////////////////////////////////////////

    extern TypeInfo E_PIVOT_TYPEINFO
    (
        E_PIVOT_ID,
        "ePivot",
        &E_TRANSFORM_TYPEINFO,
        []() -> eObject*
        {
            return new ePivot;
        }
    );

    TypeInfo* ePivot::getType() const
    {
        return &E_PIVOT_TYPEINFO;
    }

    ePivot::ePivot()
    : eTransform()
    {
        flags |= 0x00071000;
    }

    ePivot::~ePivot() {}


    ////////////////////////////////////////////////////////////////
    // ePivot serialization
    // <kao2.00486660>
    ////////////////////////////////////////////////////////////////
    void ePivot::serialize(Archive &ar)
    {
        int32_t a;

        eTransform::serialize(ar);

        if (ar.getVersion() < 0x7D)
        {
            throw ErrorMessage
            (
                "ePivot::serialize():\n" \
                "ar.version() 125 required!"
            );
        }

        /* Animations */
        animations.serialize(ar);

        /* Unknown group */
        a = 0x00;
        ar.readOrWrite(&a, 0x04);
        if (0 != a)
        {
            throw ErrorMessage
            (
                "ePivot::serialize():\n" \
                "non-empty group member is not supported!"
            );
        }
    }


    ////////////////////////////////////////////////////////////////
    // ePivot: export readable structure
    ////////////////////////////////////////////////////////////////
    void ePivot::writeStructureToTextFile(FileOperator &file, int32_t indentation) const
    {
        int32_t i;
        eNode* test_node;
        eTrack* test_track;

        char bufor[128];
        eString test_str;

        /* "eNode" parent class */

        eNode::writeStructureToTextFile(file, indentation);

        /* "ePivot" additional info */

        for (i = 0; i < animations.tracks.getSize(); i++)
        {
            test_track = (eTrack*)animations.tracks.getIthChild(i);
            sprintf_s
            (
                bufor, 128,
                " - track [%d]: \"%s\" [%.2f, %.2f]",
                i,
                test_track->getStringRepresentation().getText(),
                test_track->getStartFrame(),
                test_track->getEndFrame()
            );

            ArFunctions::writeIndentation(file, indentation);
            file << bufor;
            ArFunctions::writeNewLine(file, 0);
        }

        /* "eTransform" parent class */

        sprintf_s
        (
            bufor, 128,
            " - xform pos: (%f, %f, %f)",
            defaultTransform.pos.x,
            defaultTransform.pos.y,
            defaultTransform.pos.z
        );

        ArFunctions::writeIndentation(file, indentation);
        file << bufor;
        ArFunctions::writeNewLine(file, 0);

        sprintf_s
        (
            bufor, 128,
            " - xform rot: (%f, %f, %f, %f)",
            defaultTransform.rot.x,
            defaultTransform.rot.y,
            defaultTransform.rot.z,
            defaultTransform.rot.w
        );

        ArFunctions::writeIndentation(file, indentation);
        file << bufor;
        ArFunctions::writeNewLine(file, 0);

        sprintf_s
        (
            bufor, 128,
            " - xform scl: (%f)",
            defaultTransform.scale
        );

        ArFunctions::writeIndentation(file, indentation);
        file << bufor;
        ArFunctions::writeNewLine(file, 0);

        /* "eGroup" parent class */

        for (i = 0; i < nodes.getSize(); i++)
        {
            test_node = (eNode*)nodes.getIthChild(i);

            if (nullptr != test_node)
            {
                test_node->writeStructureToTextFile(file, (indentation + 1));
            }
        }
    }


    ////////////////////////////////////////////////////////////////
    // ePivot: set animation object for rendering
    ////////////////////////////////////////////////////////////////
    bool ePivot::renderObject(int32_t draw_flags, eAnimate* anim, eSRP &parent_srp, eMatrix4x4 &parent_matrix, int32_t marked_id)
    {
        if (GUI::drawFlags::DRAW_FLAG_ANIMS & draw_flags)
        {
            animations.setAnimation(GUI::animationID, GUI::timerGetFrames());
        }

        return eTransform::renderObject(draw_flags, &animations, parent_srp, parent_matrix, marked_id);
    }


    ////////////////////////////////////////////////////////////////
    // ePivot: rebuild "eAnimState"(s) in "eAnimate"
    ////////////////////////////////////////////////////////////////
    void ePivot::rebuildEmptyAnimState()
    {
        animations.rebuildEmptyAnimState();
    }


    ////////////////////////////////////////////////////////////////
    // ePivot: changing the number of animation tracks
    ////////////////////////////////////////////////////////////////

    void ePivot::animAddTrack(eString anim_name, float first_frame, float frames_count)
    {
        eTrack* dummy_track = new eTrack;

        if (first_frame < 0)
        {
            first_frame = 0;
        }

        if (frames_count < 0)
        {
            frames_count = (-frames_count);
        }

        dummy_track->setName(anim_name);
        dummy_track->setStartFrame(first_frame);
        dummy_track->setEndFrame(first_frame + frames_count);

        animations.tracks.appendChild(dummy_track);

        ctrlExpandAnimTracks(animations.tracks.getSize());
    }

    void ePivot::animRemoveTrack(eString anim_name)
    {
        int result = (-1);
        eTrack* dummy_track;

        for (int i = 0; (result < 0) && (i < animations.tracks.getSize()); i++)
        {
            dummy_track = (eTrack*)animations.tracks.getIthChild(i);

            if ((nullptr != dummy_track) && (dummy_track->getStringRepresentation().compareExact(anim_name, true)))
            {
                result = i;
            }
        }

        if (result >= 0)
        {
            animations.tracks.deleteIthChild(result);

            ctrlRemoveAnimTrack(result);
        }
    }

}
