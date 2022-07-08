#include <lights.h>
#include <math.h>

BaseLight::BaseLight(std::shared_ptr<RTUtil::LightInfo> l) : type(l->type){};

AmbientLight::AmbientLight(std::shared_ptr<RTUtil::LightInfo> l) : BaseLight(l)
{
    powerOrRad = l->radiance;
    radiance = l->radiance;
    if (l->range)
    {
        range = l->range;
    }
    else
    {
        range = std::numeric_limits<float>::infinity();
        // range = 0;
    }
};

Eigen::Vector3f AmbientLight::getContribution(Eigen::Vector3f incomingDir, Eigen::Vector3f intersection, Eigen::Vector3f normal, std::shared_ptr<nori::BSDF> material,
                                              std::function<bool(Eigen::Vector3f, float)> doShadowTest)
{
    // randomly sample a point on the unit s
    float x = (float)rand() / (float)RAND_MAX;
    float y = (float)rand() / (float)RAND_MAX;

    RTUtil::Point2 sample(x, y);

    Eigen::Vector3f sampledOSpace = RTUtil::squareToCosineHemisphere(sample);
    nori::Frame frame(normal);
    Eigen::Vector3f sampledWSpace = frame.toWorld(sampledOSpace);

    if (doShadowTest(intersection + 4 * sampledWSpace, range))
    {
        return radiance.cwiseProduct(material->diffuseReflectance());
    }
    else
    {
        return Eigen::Vector3f(0, 0, 0);
    }
    // Check for occlusion

    // Return MonteCarlo Estimate
}

PointLight::PointLight(std::shared_ptr<RTUtil::LightInfo> l, Eigen::Affine3f transform) : BaseLight(l)
{
    position = transform * l->position;
    power = l->power;
    std::cout << "The position of the pointlight is: " << position << '\n';
};

Eigen::Vector3f PointLight::getContribution(Eigen::Vector3f incomingDir, Eigen::Vector3f intersection, Eigen::Vector3f normal, std::shared_ptr<nori::BSDF> material,
                                            std::function<bool(Eigen::Vector3f, float)> doShadowTest)
{
    if (doShadowTest(this->position, std::numeric_limits<float>::infinity()))
    {
        // std::cout << "no shadow\n";

        Eigen::Vector3f outGoing = this->position - intersection;

        float dist = outGoing.norm();
        Eigen::Vector3f lightDir = outGoing.normalized();

        // Create a frame
        nori::Frame frame(normal);

        // Craft a BSDF query
        nori::BSDFQueryRecord query(-frame.toLocal(incomingDir), frame.toLocal(lightDir));

        // v dot w
        float x = lightDir.dot(normal) / (dist * dist);

        nori::Color3f color = material->eval(query);

        // std::cout << color << "\n";

        return x * color.cwiseProduct(power) / (4 * M_PI);
    }
    else
    {
        return Eigen::Vector3f(0, 0, 0);
    }
}

AreaLight::AreaLight(std::shared_ptr<RTUtil::LightInfo> l, Eigen::Affine3f transform) : BaseLight(l)
{
    std::cout << "The position of the arealight is: " << l->position << '\n';
    this->power = l->power;

    Eigen::Vector3f pUp = l->up.normalized();
    Eigen::Vector3f pRight = pUp.cross(l->normal).normalized();

    float hWidth = l->size.x() / 2;
    float hHeight = l->size.y() / 2;

    Eigen::Vector3f pbl = l->position - (pRight * hWidth) - (pUp * hHeight);
    Eigen::Vector3f pbr = l->position + (pRight * hWidth) - (pUp * hHeight);
    Eigen::Vector3f ptl = l->position - (pRight * hWidth) + (pUp * hHeight);
    Eigen::Vector3f ptr = l->position + (pRight * hWidth) + (pUp * hHeight);

    Eigen::Vector3f nbl = transform * pbl;
    Eigen::Vector3f nbr = transform * pbr;
    Eigen::Vector3f ntl = transform * ptl;
    Eigen::Vector3f ntr = transform * ptr;

    this->botLeft = nbl;
    this->upDir = (ntl - nbl).normalized();
    this->height = (ntl - nbl).norm();

    this->rightDir = (ntr - ntl).normalized();
    this->width = (ntr - ntl).norm();

    this->nor = (transform.linear() * l->normal).normalized();
};

Eigen::Vector3f AreaLight::getContribution(Eigen::Vector3f incomingDir, Eigen::Vector3f intersection, Eigen::Vector3f normal, std::shared_ptr<nori::BSDF> material,
                                           std::function<bool(Eigen::Vector3f, float)> doShadowTest)
{
    float prx = (float)rand() / (float)RAND_MAX;
    float rx = prx * this->width;

    float pry = (float)rand() / (float)RAND_MAX;
    float ry = pry * this->height;

    Eigen::Vector3f randPoint = this->botLeft + (this->upDir * ry) + (this->rightDir * rx);
    Eigen::Vector3f outGoing = randPoint - intersection;

    if (doShadowTest(randPoint, std::numeric_limits<float>::infinity()) & (this->nor.dot(outGoing) < 0))
    {

        float dist = outGoing.norm();
        Eigen::Vector3f lightDir = outGoing.normalized();

        // Create a frame
        nori::Frame frame(normal);

        // Craft a BSDF query
        nori::BSDFQueryRecord query(frame.toLocal(-incomingDir).normalized(), frame.toLocal(lightDir).normalized());
        nori::Color3f bsdf = material->eval(query);

        Eigen::Vector3f L = power / (2 * M_PI * width * height);

        float rest = height * width * lightDir.dot(normal) * lightDir.dot(this->nor) / (dist * dist);
        rest = abs(rest);

        // std::cout << bsdf << '\n';

        return rest * bsdf.cwiseProduct(L);
    }
    else
    {
        return Eigen::Vector3f(0, 0, 0);
    }
}
