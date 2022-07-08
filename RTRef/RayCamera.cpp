#include <RayCamera.h>
#include <math.h>

RayCamera::RayCamera()
{
    this->eye = Eigen::Vector3f(3, 4, 5);
    this->lookDir = Eigen::Vector3f(0, 0, 0);
    this->upDir = Eigen::Vector3f(0, 1, 0);

    this->aspectRatio = 1.f;
    this->near = 0;
    this->far = std::numeric_limits<float>::infinity();
    this->hfov = 30.f;

    this->vw = (eye - lookDir).normalized();
    this->vv = upDir.normalized();
    this->vu = vv.cross(vw);
}

RayCamera::RayCamera(
    Eigen::Vector3f position,
    Eigen::Vector3f target,
    Eigen::Vector3f up,
    float hfov,
    float aspectRatio,
    float near,
    float far) : eye(position), lookDir(target), upDir(up), hfov(hfov), aspectRatio(aspectRatio), near(near), far(far)
{

    this->vw = (-lookDir).normalized();
    this->vv = upDir.normalized();
    this->vu = vv.cross(vw);
}

void RayCamera::orbit(float theta, float phi)
{
    const Eigen::Vector3f up(0., 1., 0.);
    Eigen::AngleAxisf xRot(theta, up);

    this->eye = xRot * this->eye;
    this->vw = xRot * this->vw;
    this->vu = xRot * this->vu;
    this->vv = xRot * this->vv;

    Eigen::AngleAxisf yRot(phi, this->vu);

    this->eye = yRot * this->eye;
    this->vw = yRot * this->vw;
    this->vv = yRot * this->vv;
}

void RayCamera::zoom(float z)
{
    this->eye = this->eye + z * this->vw;
}

Eigen::Vector3f RayCamera::getvw()
{
    return this->vw;
}

RTCRay RayCamera::generateRay(float u, float v)
{

    float width = abs(2 * tan(hfov / 2));

    float height = abs(width / aspectRatio);

    // printf("The width and height are: %f, %f \n", width, height);

    float cu = (width * u) - (width / 2);
    float cv = (height * v) - (height / 2);

    Eigen::Vector3f dir = (vw * -1) + (vu * cu) + (vv * cv);

    struct RTCRay ray;
    ray.org_x = eye.x();
    ray.org_y = eye.y();
    ray.org_z = eye.z();
    ray.dir_x = dir.x();
    ray.dir_y = dir.y();
    ray.dir_z = dir.z();
    ray.tnear = 0;
    ray.tfar = std::numeric_limits<float>::infinity();
    ray.mask = 0;
    ray.flags = 0;

    return ray;
}

float RayCamera::getAspect() { return aspectRatio; }