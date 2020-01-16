#include <kao2engine/eTriMesh.h>

#include <kao2engine/eGeoSet.h>
#include <kao2engine/ePhyTriMesh.h>
#include <kao2engine/eTransform.h>

#include <kao2engine/eBitmap.h>
#include <kao2engine/eTexture.h>
#include <kao2engine/eMaterial.h>

#include <utilities/ColladaExporter.h>

namespace ZookieWizard
{

    ////////////////////////////////////////////////////////////////
    // eTriMesh: COLLADA exporting
    ////////////////////////////////////////////////////////////////
    void eTriMesh::writeNodeToXmlFile(ColladaExporter &exporter)
    {
        int32_t i, j, k;
        int32_t geo_id, mat_id;
        char bufor[64];
        
        eTransform* test_xform;
        eBitmap* test_bitmap;
        eTexture* test_texture;
        ePhyTriMesh* test_phytrimesh;

        int32_t array_length = 0;
        ePoint4* array_data4 = nullptr;
        ePoint2* array_data2 = nullptr;
        int32_t total_indices;
        int32_t total_triangles;
        eGeoArray<ePoint4>* vertices = geo->getVerticesArray();
        eGeoArray<ePoint4>* normals =  geo->getNormalsArray();
        eGeoArray<ePoint2>* mapping = geo->getTextureCoordsArray();
        eGeoArray<ushort>* indices_offsets = geo->getIndicesOffsets();
        eGeoArray<ushort>* indices = geo->getIndicesArray();

        const char* array_names[3] = {"positions", "normals", "mapping"};
        const char* semantic_names[6] = {"VERTEX", "NORMAL", "TEXCOORD", "vertices", "normals", "mapping"};
        void* array_pointers[3] = {vertices, normals, mapping};
        const char* accessor_params[5] = {"X", "Y", "Z", "S", "T"};

        switch (exporter.getState())
        {
            case COLLADA_EXPORTER_STATE_IMAGES:
            {
                if (nullptr != material)
                {
                    test_texture = material->getIthTexture(0);

                    if (nullptr != test_texture)
                    {
                        test_bitmap = test_texture->getBitmap();

                        if (nullptr != test_bitmap)
                        {
                            test_bitmap->writeNodeToXmlFile(exporter);
                        }
                    }
                }

                break;
            }

            case COLLADA_EXPORTER_STATE_EFFECTS:
            {
                if (nullptr != material)
                {
                    test_texture = material->getIthTexture(0);

                    if (nullptr != test_texture)
                    {
                        test_texture->writeNodeToXmlFile(exporter);
                    }
                }

                break;
            }

            case COLLADA_EXPORTER_STATE_CONTROLLERS:
            {
                if (nullptr != geo)
                {
                    test_phytrimesh = geo->getPhyTriMesh();

                    if (nullptr != test_phytrimesh)
                    {
                        test_phytrimesh->writeNodeToXmlFile(exporter);
                    }
                }

                break;
            }

            case COLLADA_EXPORTER_STATE_MATERIALS:
            {
                if (exporter.objectRefAlreadyExists(COLLADA_EXPORTER_OBJ_MATERIAL, material))
                {
                    /* Material was already exported */
                    return;
                }

                if (nullptr != material)
                {
                    exporter.openTag("material");

                    mat_id = exporter.getObjectRefId(COLLADA_EXPORTER_OBJ_MATERIAL, material, true);
                    sprintf_s(bufor, 64, "Material%d", mat_id);
                    exporter.insertTagAttrib("id", bufor);
                    exporter.insertTagAttrib("name", material->getStringRepresentation());

                    test_texture = material->getIthTexture(0);

                    if (nullptr != test_texture)
                    {
                        exporter.openTag("instance_effect");

                        i = exporter.getObjectRefId(COLLADA_EXPORTER_OBJ_EFFECT, test_texture, false);
                        sprintf_s(bufor, 64, "#Texture%d", i);
                        exporter.insertTagAttrib("url", bufor);

                        exporter.closeTag(); // "instance_effect"
                    }

                    exporter.closeTag(); // "material"
                }

                break;
            }

            case COLLADA_EXPORTER_STATE_GEOMETRIES:
            {
                if (exporter.objectRefAlreadyExists(COLLADA_EXPORTER_OBJ_GEOMETRY, geo))
                {
                    /* TriMesh was already exported */
                    return;
                }

                exporter.openTag("geometry");

                geo_id = exporter.getObjectRefId(COLLADA_EXPORTER_OBJ_GEOMETRY, geo, true);
                sprintf_s(bufor, 64, "TriMesh%d", geo_id);
                exporter.insertTagAttrib("id", bufor);
                exporter.insertTagAttrib("name", name);

                exporter.openTag("mesh");

                /********************************/
                /* Write vertrices data */

                for (i = 0; i < 3; i++)
                {
                    if (nullptr != array_pointers[i])
                    {
                        exporter.openTag("source");

                        sprintf_s(bufor, 64, "TriMesh%d-%s", geo_id, array_names[i]);
                        exporter.insertTagAttrib("id", bufor);

                        exporter.openTag("float_array");

                        sprintf_s(bufor, 64, "TriMesh%d-%s-array", geo_id, array_names[i]);
                        exporter.insertTagAttrib("id", bufor);

                        switch (i)
                        {
                            case 0: // VERTEX POSITIONS
                            {
                                array_length = vertices->getLength();
                                array_data4 = vertices->getData();
                                k = 3;

                                break;
                            }

                            case 1: // VERTEX NORMALS
                            {
                                array_length = normals->getLength();
                                array_data4 = normals->getData();
                                k = 3;

                                break;
                            }

                            case 2: // VERTEX UV MAPPING
                            {
                                array_length = mapping->getLength();
                                array_data2 = mapping->getData();
                                k = 2;

                                break;
                            }
                        }

                        sprintf_s(bufor, 64, "%d", (k * array_length));
                        exporter.insertTagAttrib("count", bufor);

                        switch (i)
                        {
                            case 0: // VERTEX POSITIONS
                            case 1: // VERTEX NORMALS
                            {
                                for (j = 0; j < array_length; j++)
                                {
                                    sprintf_s
                                    (
                                        bufor, 64, "%f %f %f",
                                        array_data4[j].x, array_data4[j].y, array_data4[j].z
                                    );

                                    exporter.writeInsideTag(bufor);
                                }

                                break;
                            }

                            case 2: // VERTEX UV MAPPING
                            {
                                for (j = 0; j < array_length; j++)
                                {
                                    /* Flip texture mapping vertically */

                                    sprintf_s
                                    (
                                        bufor, 64, "%f %f",
                                        array_data2[j].u, (1.0f - array_data2[j].v)
                                    );

                                    exporter.writeInsideTag(bufor);
                                }

                                break;
                            }
                        }

                        exporter.closeTag(); // "float_array"

                        exporter.openTag("technique_common");

                        exporter.openTag("accessor");

                        sprintf_s(bufor, 64, "#TriMesh%d-%s-array", geo_id, array_names[i]);
                        exporter.insertTagAttrib("source", bufor);
                        sprintf_s(bufor, 64, "%d", array_length);
                        exporter.insertTagAttrib("count", bufor);
                        sprintf_s(bufor, 64, "%d", k);
                        exporter.insertTagAttrib("stride", bufor);

                        for (j = 0; j < k; j++)
                        {
                            exporter.openTag("param");
                            exporter.insertTagAttrib("name", accessor_params[(i < 2 ? 0 : 3) + j]);
                            exporter.insertTagAttrib("type", "float");
                            exporter.closeTag();
                        }

                        exporter.closeTag(); // "accessor"

                        exporter.closeTag(); // "technique_common"

                        exporter.closeTag(); // "source"
                    }
                }

                /********************************/
                /* Write indices data */

                exporter.openTag("vertices");

                sprintf_s(bufor, 64, "TriMesh%d-vertices", geo_id);
                exporter.insertTagAttrib("id", bufor);

                exporter.openTag("input");
                exporter.insertTagAttrib("semantic", "POSITION");
                sprintf_s(bufor, 64, "#TriMesh%d-positions", geo_id);
                exporter.insertTagAttrib("source", bufor);
                exporter.closeTag(); // "input"

                exporter.closeTag(); // "vertices"

                exporter.openTag("triangles"); // "tristrips" are unsupported

                if (nullptr != material)
                {
                    mat_id = exporter.getObjectRefId(COLLADA_EXPORTER_OBJ_MATERIAL, material, false);
                    sprintf_s(bufor, 64, "Material%d", mat_id);
                    exporter.insertTagAttrib("material", bufor);
                }

                total_triangles = 0;

                if (nullptr != indices_offsets)
                {
                    for (i = 0; i < indices_offsets->getLength(); i++)
                    {
                        total_triangles += (indices_offsets->getData()[i] - 2);
                    }
                }
                else if (nullptr != indices)
                {
                    total_triangles += (indices->getLength() - 2);
                }

                sprintf_s(bufor, 64, "%d", total_triangles);
                exporter.insertTagAttrib("count", bufor);

                for (i = 0; i < 3; i++)
                {
                    exporter.openTag("input");
                    exporter.insertTagAttrib("semantic", semantic_names[i]);
                    sprintf_s(bufor, 64, "#TriMesh%d-%s", geo_id, semantic_names[3 + i]);
                    exporter.insertTagAttrib("source", bufor);
                    exporter.insertTagAttrib("offset", "0");
                    if (i >= 2)
                    {
                        exporter.insertTagAttrib("set", "0");
                    }
                    exporter.closeTag(); // "input"
                }

                if (nullptr != indices_offsets)
                {
                    if (nullptr != indices)
                    {
                        total_indices = 0;

                        exporter.openTag("p");

                        for (i = 0; i < indices_offsets->getLength(); i++)
                        {
                            for (j = 0; j < (indices_offsets->getData()[i] - 2); j++)
                            {
                                k = indices->getData()[total_indices + j];
                                sprintf_s(bufor, 64, "%d", k);
                                exporter.writeInsideTag(bufor);

                                k = indices->getData()[total_indices + j + 1 + (j % 2)];
                                sprintf_s(bufor, 64, "%d", k);
                                exporter.writeInsideTag(bufor);

                                k = indices->getData()[total_indices + j + 1 + 1 - (j % 2)];
                                sprintf_s(bufor, 64, "%d", k);
                                exporter.writeInsideTag(bufor);
                            }

                            total_indices += indices_offsets->getData()[i];
                        }

                        exporter.closeTag(); // "p"
                    }
                }
                else if (nullptr != indices)
                {
                    /* (--dsp--) */

                    exporter.openTag("p");

                    for (j = 0; j < indices->getLength(); j++)
                    {
                        k = indices->getData()[j];

                        sprintf_s(bufor, 64, "%d %d %d", k, k, k);
                        exporter.writeInsideTag(bufor);
                    }

                    exporter.closeTag(); // "p"
                }

                exporter.closeTag(); // "triangles"

                /********************************/
                /* Close "eTriMesh" */

                exporter.closeTag(); // "mesh"

                exporter.closeTag(); // "geometry"

                break;
            }

            case COLLADA_EXPORTER_STATE_VISUAL_SCENES:
            {
                exporter.openTag("node");

                i = exporter.getObjectRefId(COLLADA_EXPORTER_OBJ_NODE, this, true);
                sprintf_s(bufor, 64, "Node%d", i);
                exporter.insertTagAttrib("id", bufor);
                exporter.insertTagAttrib("name", name);

                k = 0;

                if (nullptr != geo)
                {
                    test_phytrimesh = geo->getPhyTriMesh();

                    if (nullptr != test_phytrimesh)
                    {
                        test_xform = test_phytrimesh->getArmatureParent();

                        if (nullptr != test_xform)
                        {
                            k = 1;

                            exporter.openTag("instance_controller");

                            i = exporter.getObjectRefId(COLLADA_EXPORTER_OBJ_ARMATURE, test_phytrimesh, false);
                            sprintf_s(bufor, 64, "#Armature%d", i);
                            exporter.insertTagAttrib("url", bufor);

                            exporter.openTag("skeleton");

                            i = exporter.getObjectRefId(COLLADA_EXPORTER_OBJ_NODE, test_xform, false);
                            sprintf_s(bufor, 64, "#Node%d", i);
                            exporter.writeInsideTag(bufor);

                            exporter.closeTag(); // "skeleton"
                        }
                    }
                }

                if (0 == k)
                {
                    exporter.openTag("instance_geometry");

                    geo_id = exporter.getObjectRefId(COLLADA_EXPORTER_OBJ_GEOMETRY, geo, false);
                    sprintf_s(bufor, 64, "#TriMesh%d", geo_id);
                    exporter.insertTagAttrib("url", bufor);
                }

                if (nullptr != material)
                {
                    exporter.openTag("bind_material");
                    exporter.openTag("technique_common");
                    exporter.openTag("instance_material");

                    mat_id = exporter.getObjectRefId(COLLADA_EXPORTER_OBJ_MATERIAL, material, false);
                    sprintf_s(bufor, 64, "#Material%d", mat_id);
                    exporter.insertTagAttrib("target", bufor);
                    sprintf_s(bufor, 64, "Material%d", mat_id);
                    exporter.insertTagAttrib("symbol", bufor);

                    exporter.openTag("bind_vertex_input");
                    exporter.insertTagAttrib("semantic", "UVMap");
                    exporter.insertTagAttrib("input_semantic", "TEXCOORD");
                    exporter.insertTagAttrib("input_set", "0");

                    exporter.closeTag(); // "bind_vertex_input"
                    exporter.closeTag(); // "instance_material"
                    exporter.closeTag(); // "technique_common"
                    exporter.closeTag(); // "bind_material"
                }

                exporter.closeTag(); // "instance_geometry" or "instance_controller"
                exporter.closeTag(); // "node"

                break;
            }
        }
    }

}