#include <ext/embree/include/embree3/rtcore.h>
#include <iostream>
#include <stdio.h>
#include <math.h>
#include <limits>
#include <typeinfo>
#include <RayCamera.h>
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <RTUtil/conversions.hpp>

#include <ext/assimp/include/assimp/Importer.hpp>  // Plain-C interface
#include <ext/assimp/include/assimp/scene.h>       // Output data structure
#include <ext/assimp/include/assimp/postprocess.h> // Post processing flags

#include <lights.h>
using namespace std;

class SceneAndCam
{
public:
  RTCDevice device;
  RTCScene scene;
  RayCamera cam;
  RTUtil::SceneInfo info;
  vector<shared_ptr<BaseLight>> lights;
  int numMeshes = 0;
  map<int, shared_ptr<nori::BSDF>> materials;
  shared_ptr<nori::BSDF> defaultMat;
};

class Generator
{

  static void initializeCamera(shared_ptr<SceneAndCam> sc, const aiScene *data);

  static void initializeScene(shared_ptr<SceneAndCam> sc, const aiScene *data);

  static void initializeSceneHelper(shared_ptr<SceneAndCam> sc, const aiScene *data, aiNode *node, Eigen::Affine3f prevTransform);
  static void traverseScene(const aiScene *data);

  static void initializeDevice(shared_ptr<SceneAndCam> sc);

public:
  static shared_ptr<SceneAndCam> generateScene(const string &filename);
};
