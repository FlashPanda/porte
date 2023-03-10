#ifndef TINY_OBJ_LOADER_H_
#define TINY_OBJ_LOADER_H_

#include <string>
#include <vector>
#include <map>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cctype>

#include <fstream>
#include <sstream>
#include <core/porte.h>

namespace porte
{
    typedef struct {
        std::string name;

        float ambient[3];
        float diffuse[3];
        float specular[3];
        float transmittance[3];
        float emission[3];
        float shininess;
        float ior;      // index of refraction
        float dissolve; // 1 == opaque; 0 == fully transparent
        // illumination model (see http://www.fileformat.info/format/material/)
        int illum;

        int dummy; // Suppress padding warning.

        std::string ambient_texname;            // map_Ka
        std::string diffuse_texname;            // map_Kd
        std::string specular_texname;           // map_Ks
        std::string specular_highlight_texname; // map_Ns
        std::string bump_texname;               // map_bump, bump
        std::string displacement_texname;       // disp
        std::string alpha_texname;              // map_d
        std::map<std::string, std::string> unknown_parameter;
    } material_t;

    typedef struct {
        std::string name;

        std::vector<int> intValues;
        std::vector<float> floatValues;
        std::vector<std::string> stringValues;
    } tag_t;

    typedef struct {
        std::vector<float> positions;
        std::vector<float> normals;
        std::vector<float> texcoords;
        std::vector<unsigned int> indices;
        std::vector<unsigned char>
            num_vertices;              // The number of vertices per face. Up to 255.
        std::vector<int> material_ids; // per-face material ID
        std::vector<tag_t> tags;       // SubD tag
    } mesh_t;

    typedef struct {
        std::string name;
        mesh_t mesh;
    } shape_t;

    typedef enum
    {
        triangulation = 1,        // used whether triangulate polygon face in .obj
        calculate_normals = 2,    // used whether calculate the normals if the .obj normals are empty
        // Some nice stuff here
    } load_flags_t;

    class float3
    {
    public:
        float3()
            : x(0.0f)
            , y(0.0f)
            , z(0.0f)
        {
        }

        float3(float coord_x, float coord_y, float coord_z)
            : x(coord_x)
            , y(coord_y)
            , z(coord_z)
        {
        }

        float3(const float3& from, const float3& to)
        {
            coord[0] = to.coord[0] - from.coord[0];
            coord[1] = to.coord[1] - from.coord[1];
            coord[2] = to.coord[2] - from.coord[2];
        }

        float3 crossproduct(const float3& vec)
        {
            float a = y * vec.z - z * vec.y;
            float b = z * vec.x - x * vec.z;
            float c = x * vec.y - y * vec.x;
            return float3(a, b, c);
        }

        void normalize()
        {
            const float length = std::sqrt((coord[0] * coord[0]) +
                (coord[1] * coord[1]) +
                (coord[2] * coord[2]));
            if (length != 1)
            {
                coord[0] = (coord[0] / length);
                coord[1] = (coord[1] / length);
                coord[2] = (coord[2] / length);
            }
        }

    private:
        union
        {
            float coord[3];
            struct
            {
                float x, y, z;
            };
        };
    };

    class MaterialReader {
    public:
        MaterialReader() {}
        virtual ~MaterialReader();

        virtual bool operator()(const std::string& matId,
            std::vector<material_t>& materials,
            std::map<std::string, int>& matMap,
            std::string& err) = 0;
    };

    class MaterialFileReader : public MaterialReader {
    public:
        MaterialFileReader(const std::string& mtl_basepath)
            : m_mtlBasePath(mtl_basepath) {}
        virtual ~MaterialFileReader() {}
        virtual bool operator()(const std::string& matId,
            std::vector<material_t>& materials,
            std::map<std::string, int>& matMap, std::string& err);

    private:
        std::string m_mtlBasePath;
    };

    /// Loads .obj from a file.
    /// 'shapes' will be filled with parsed shape data
    /// The function returns error string.
    /// Returns true when loading .obj become success.
    /// Returns warning and error message into `err`
    /// 'mtl_basepath' is optional, and used for base path for .mtl file.
    /// 'optional flags
    bool LoadObj(std::vector<shape_t>& shapes,       // [output]
        std::vector<material_t>& materials, // [output]
        std::string& err,                   // [output]
        const char* filename, const char* mtl_basepath = NULL,
        unsigned int flags = 1);

    /// Loads object from a std::istream, uses GetMtlIStreamFn to retrieve
    /// std::istream for materials.
    /// Returns true when loading .obj become success.
    /// Returns warning and error message into `err`
    bool LoadObj(std::vector<shape_t>& shapes,       // [output]
        std::vector<material_t>& materials, // [output]
        std::string& err,                   // [output]
        std::istream& inStream, MaterialReader& readMatFn,
        unsigned int flags = 1);

    /// Loads materials into std::map
    void LoadMtl(std::map<std::string, int>& material_map, // [output]
        std::vector<material_t>& materials,       // [output]
        std::istream& inStream);

    std::vector<std::shared_ptr<Shape>> ObjLoader(std::string fullpath, const Transform* trans, const Transform* invTrans);



}

#endif // TINY_OBJ_LOADER_H_