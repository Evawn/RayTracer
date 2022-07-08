#include <generator.h>
using namespace std;

string resourcePath = "../resources/scenes/";
string infoFile = "_info.json";
string meshFile = ".dae";

void Generator::initializeScene(shared_ptr<SceneAndCam> sc, const aiScene *data)
{

    RTCScene scene = rtcNewScene(sc->device);
    sc->scene = scene;

    // Add all ambient lights
    for (int i = 0; i < sc->info.lights.size(); i++)
    {
        std::shared_ptr<RTUtil::LightInfo> l = sc->info.lights[i];

        if (l->type == RTUtil::Ambient)
        {
            shared_ptr<AmbientLight> light = make_shared<AmbientLight>(l);
            sc->lights.push_back(light);
            std::cout << "Added an ambient light \n";
        }
    }

    aiNode *node = data->mRootNode;
    Eigen::Affine3f transform = RTUtil::a2e(node->mTransformation);

    initializeSceneHelper(
        sc,
        data,
        node,
        Eigen::Affine3f::Identity());

    rtcCommitScene(scene);
}

void Generator::initializeSceneHelper(shared_ptr<SceneAndCam> sc, const aiScene *data, aiNode *node, Eigen::Affine3f prevTransform)
{

    // Matrix that takes from object to world
    Eigen::Affine3f transform = prevTransform * RTUtil::a2e(node->mTransformation);

    // Gather and add all of the meshes
    for (int m = 0; m < node->mNumMeshes; m++)
    {

        unsigned int meshNum = node->mMeshes[m];
        aiMesh *mesh = data->mMeshes[meshNum];

        int nVerts = mesh->mNumVertices;
        int nFaces = mesh->mNumFaces;

        RTCGeometry geom = rtcNewGeometry(sc->device, RTC_GEOMETRY_TYPE_TRIANGLE);

        float *vertices = (float *)rtcSetNewGeometryBuffer(
            geom,
            RTC_BUFFER_TYPE_VERTEX,
            0,
            RTC_FORMAT_FLOAT3,
            3 * sizeof(float),
            nVerts);

        unsigned *indices = (unsigned *)rtcSetNewGeometryBuffer(
            geom,
            RTC_BUFFER_TYPE_INDEX,
            0,
            RTC_FORMAT_UINT3,
            3 * sizeof(unsigned),
            nFaces);

        if (vertices && indices)
        {
            for (int i = 0; i < nVerts; i++)
            {

                Eigen::Vector3f vert = RTUtil::a2e(mesh->mVertices[i]);
                vert = transform * vert;

                vertices[3 * i] = vert.x();
                vertices[3 * i + 1] = vert.y();
                vertices[3 * i + 2] = vert.z();
            }

            for (int i = 0; i < nFaces; i++)
            {
                indices[3 * i] = mesh->mFaces[i].mIndices[0];
                indices[3 * i + 1] = mesh->mFaces[i].mIndices[1];
                indices[3 * i + 2] = mesh->mFaces[i].mIndices[2];
            }
        }

        rtcCommitGeometry(geom);
        rtcAttachGeometry(sc->scene, geom);
        rtcReleaseGeometry(geom);

        // Add the material for the geometry
        // If the mesh has a material and there is a BSDF in the scene info with a "name" field matching the material name, use it.
        // Otherwise, if the mesh is below a node whose name matches the "node" field of a material in the scene info, use it.
        // Otherwise use the scene's default material.
        aiString meshMat = data->mMaterials[mesh->mMaterialIndex]->GetName();
        try
        {
            std::shared_ptr<nori::BSDF> mat = sc->info.namedMaterials.at(meshMat.C_Str());
            sc->materials.insert(std::make_pair(sc->numMeshes, mat));
        }
        catch (...)
        {
            try
            {
                std::shared_ptr<nori::BSDF> mat = sc->info.nodeMaterials.at(node->mName.C_Str());
                sc->materials.insert(std::make_pair(sc->numMeshes, mat));
            }
            catch (...)
            {
                std::shared_ptr<nori::BSDF> mat = sc->info.defaultMaterial;
                sc->materials.insert(std::make_pair(sc->numMeshes, mat));
            }
        }

        sc->numMeshes++;
    }

    // Parse through the scene info
    // Look for lights under node
    for (int i = 0; i < sc->info.lights.size(); i++)
    {
        std::shared_ptr<RTUtil::LightInfo> l = sc->info.lights[i];

        if (node->mName.C_Str() == l->nodeName)
        {
            if (l->type == RTUtil::Point)
            {
                shared_ptr<PointLight> light = make_shared<PointLight>(l, transform);
                sc->lights.push_back(light);
                std::cout << "Added a point light \n";
            }
            else if (l->type == RTUtil::Area)
            {
                shared_ptr<AreaLight> light = make_shared<AreaLight>(l, transform);
                sc->lights.push_back(light);
                std::cout << "Added an area light \n";
            }
        }
    }

    // Recurse through the children nodes
    for (int c = 0; c < node->mNumChildren; c++)
    {
        initializeSceneHelper(
            sc,
            data,
            node->mChildren[c],
            transform);
    }

    return;
}

void Generator::traverseScene(const aiScene *data)
{

    printf("This scene has %u meshes! \n", data->mNumMeshes);

    for (int m = 0; m < data->mNumMeshes; m++)
    {
        unsigned int nVerts = data->mMeshes[m]->mNumVertices;
        unsigned int nFaces = data->mMeshes[m]->mNumFaces;

        printf("Mesh %i has %u vertices and %u faces!\n", m, nVerts, nFaces);
    }
}
/* Camera initializing ---------------------------------------------------------------------------------*/
void Generator::initializeCamera(shared_ptr<SceneAndCam> sc, const aiScene *data)
{

    if (data->HasCameras())
    {

        aiCamera *camData = data->mCameras[0];
        aiString camNodeName = camData->mName;

        aiNode *curNode = data->mRootNode->FindNode(camNodeName);

        Eigen::IOFormat format(Eigen::StreamPrecision, 0, ", ", "; ", "", "", "[", "]\n");

        Eigen::Affine3f M = Eigen::Affine3f::Identity();
        while (curNode != NULL)
        {
            M = RTUtil::a2e(curNode->mTransformation) * M;
            curNode = curNode->mParent;
        }

        Eigen::Vector3f camPosWorld = M * RTUtil::a2e(camData->mPosition);
        Eigen::Vector3f camLookAtWorld = M.linear() * RTUtil::a2e(camData->mLookAt);
        Eigen::Vector3f camUpWorld = M.linear() * RTUtil::a2e(camData->mUp);

        RayCamera cam(
            camPosWorld,
            camLookAtWorld,
            camUpWorld,
            camData->mHorizontalFOV,
            camData->mAspect,
            camData->mClipPlaneNear,
            camData->mClipPlaneFar);

        sc->cam = cam;
    }
    else
    {
        RayCamera cam;
        sc->cam = cam;
    }
}

void errorFunction(void *userPtr, enum RTCError error, const char *str)
{
    printf("error %d: %s\n", error, str);
}

void Generator::initializeDevice(shared_ptr<SceneAndCam> sc)
{
    RTCDevice device = rtcNewDevice(NULL);

    if (!device)
        printf("error %d: cannot create device\n", rtcGetDeviceError(NULL));

    rtcSetDeviceErrorFunction(device, errorFunction, NULL);

    sc->device = device;
}

/* ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- */
shared_ptr<SceneAndCam> Generator::generateScene(const string &filename)
{
    string base = filename.substr(0, filename.find('.'));
    cout << base << '\n';

    shared_ptr<SceneAndCam> sceneCam = make_shared<SceneAndCam>();

    RTUtil::SceneInfo info;
    std::cout << RTUtil::readSceneInfo(resourcePath + base + infoFile, info);
    sceneCam->info = info;

    sceneCam->defaultMat = info.defaultMaterial;

    // Create the device
    initializeDevice(sceneCam);

    // Import the scene files
    Assimp::Importer importer;
    const aiScene *data = importer.ReadFile(
        resourcePath + filename,
        aiProcess_CalcTangentSpace |
            aiProcess_Triangulate |
            aiProcess_JoinIdenticalVertices |
            aiProcess_SortByPType);

    if (!data)
    {
        printf("error! oh no!\n");
    }
    else
    {
        printf("Scene imported successfully\n");
    }

    traverseScene(data);

    initializeCamera(sceneCam, data);
    initializeScene(sceneCam, data);

    importer.FreeScene();
    return sceneCam;
}
