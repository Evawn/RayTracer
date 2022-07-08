#include <RTUtil/sceneinfo.hpp>
#include <Eigen/Geometry>
#include <RTUtil/microfacet.hpp>
#include <RTUtil/frame.hpp>
#include <../ext/embree/include/embree3/rtcore.h>
// Class to represent geometry-less ambient lighting

class BaseLight
{

public:
    RTUtil::LightType type;
    Eigen::Vector3f powerOrRad;
    virtual Eigen::Vector3f getContribution(Eigen::Vector3f incomingDir, Eigen::Vector3f intersection, Eigen::Vector3f normal, std::shared_ptr<nori::BSDF> material,
                                            std::function<bool(Eigen::Vector3f, float)> doShadowTest) = 0;
    BaseLight(std::shared_ptr<RTUtil::LightInfo> l);
};

class AmbientLight : public BaseLight
{
    Eigen::Vector3f radiance;
    float range;

public:
    AmbientLight(std::shared_ptr<RTUtil::LightInfo> l);
    Eigen::Vector3f getContribution(Eigen::Vector3f incomingDir, Eigen::Vector3f intersection, Eigen::Vector3f normal, std::shared_ptr<nori::BSDF> material,
                                    std::function<bool(Eigen::Vector3f, float)> doShadowTest);
};

// Class to represent a point light
class PointLight : public BaseLight
{
public:
    Eigen::Vector3f power;
    Eigen::Vector3f position;
    PointLight(std::shared_ptr<RTUtil::LightInfo> l, Eigen::Affine3f transform);
    Eigen::Vector3f getContribution(Eigen::Vector3f incomingDir, Eigen::Vector3f intersection, Eigen::Vector3f normal, std::shared_ptr<nori::BSDF> material,
                                    std::function<bool(Eigen::Vector3f, float)> doShadowTest);
};

// Class to represent an area light
class AreaLight : public BaseLight
{

public:
    Eigen::Vector3f power;

    Eigen::Vector3f upDir;
    Eigen::Vector3f nor;
    Eigen::Vector3f rightDir;
    Eigen::Vector3f botLeft;
    float width;
    float height;

    AreaLight(std::shared_ptr<RTUtil::LightInfo> l, Eigen::Affine3f transform);
    Eigen::Vector3f getContribution(Eigen::Vector3f incomingDir, Eigen::Vector3f intersection, Eigen::Vector3f normal, std::shared_ptr<nori::BSDF> material,
                                    std::function<bool(Eigen::Vector3f, float)> doShadowTest);
};