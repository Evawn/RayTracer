#include <app.h>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_MSC_SECURE_CRT
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <ext/stb/stb_image.h>
#include <ext/stb/stb_image_write.h>

float toSRGB(float c)
{
  float a = 0.055;
  if (c <= 0)
    return 0;
  else if (c < 0.0031308)
  {
    return 12.92 * c;
  }
  else
  {
    if (c >= 1.0)
      return 1.0;
    else
      return (1.0 + a) * pow(c, 1.0 / 2.4) - a;
  }
}

MyGUI::MyGUI(string name, int width, int height, shared_ptr<SceneAndCam> s) : ImgGUI(width, height)
{
  saveName = "output/render_" + name + "_";
  sceneCam = s;
  theta = 0;
  phi = 0;
  deltaZoom = 0;
  samples = 0;
  pixels = new unsigned char[windowWidth * windowHeight * 3];

  missColor = s->info.backgroundRadiance;
};

MyGUI::~MyGUI(){};

void MyGUI::computeImage()
{
  samples++;
  this->sceneCam->cam.orbit(this->theta, this->phi);
  this->sceneCam->cam.zoom(this->deltaZoom);
  this->deltaZoom = 0;
  this->phi = 0;
  this->theta = 0;

  for (int i = windowHeight - 1; i >= 0; i--)
  {
    for (int j = 0; j < windowWidth; j++)
    {
      float u = (j + 0.5) / windowWidth;
      float v = (i + 0.5) / windowHeight;
      RTCRay ray = this->sceneCam->cam.generateRay(u, v);
      Eigen::Vector3f incomingRay(ray.dir_x, ray.dir_y, ray.dir_z);

      RTCRayHit rayhit = castRay(ray, false);
      Eigen::Vector3f color(0, 0, 0);

      if (rayhit.hit.geomID != RTC_INVALID_GEOMETRY_ID)
      {

        // DO I NEED TO INCLUDE THE ORIGIN??
        Eigen::Vector3f intersection(
            rayhit.ray.org_x + (rayhit.ray.tfar * rayhit.ray.dir_x),
            rayhit.ray.org_y + (rayhit.ray.tfar * rayhit.ray.dir_y),
            rayhit.ray.org_z + (rayhit.ray.tfar * rayhit.ray.dir_z));

        Eigen::Vector3f incomingDir(
            rayhit.ray.tfar * rayhit.ray.dir_x,
            rayhit.ray.tfar * rayhit.ray.dir_y,
            rayhit.ray.tfar * rayhit.ray.dir_z);
        incomingDir.normalize();

        Eigen::Vector3f norm(rayhit.hit.Ng_x, rayhit.hit.Ng_y, rayhit.hit.Ng_z);
        if (norm.dot(incomingRay) > 0)
        {
          norm = norm * -1;
        }
        norm.normalize();

        color += MyGUI::computeShading(incomingDir, intersection, norm, sceneCam->materials.at(rayhit.hit.geomID));
      }
      else
      {
        color += missColor;
      }

      if (samples == 1)
      {
        img_data[3 * (windowWidth * i + j) + 0] = color.x();
        img_data[3 * (windowWidth * i + j) + 1] = color.y();
        img_data[3 * (windowWidth * i + j) + 2] = color.z();
      }
      else
      {
        img_data[3 * (windowWidth * i + j) + 0] = (img_data[3 * (windowWidth * i + j) + 0] * (samples - 1) + color.x()) / samples;
        img_data[3 * (windowWidth * i + j) + 1] = (img_data[3 * (windowWidth * i + j) + 1] * (samples - 1) + color.y()) / samples;
        img_data[3 * (windowWidth * i + j) + 2] = (img_data[3 * (windowWidth * i + j) + 2] * (samples - 1) + color.z()) / samples;
      }

      if (samples % 64 == 0)
      {

        pixels[3 * (windowWidth * (windowHeight - i - 1) + j) + 0] = int(toSRGB(img_data[3 * (windowWidth * i + j) + 0]) * 255.0);
        pixels[3 * (windowWidth * (windowHeight - i - 1) + j) + 1] = int(toSRGB(img_data[3 * (windowWidth * i + j) + 1]) * 255.0);
        pixels[3 * (windowWidth * (windowHeight - i - 1) + j) + 2] = int(toSRGB(img_data[3 * (windowWidth * i + j) + 2]) * 255.0);
      }
    }
  }

  if (samples % 64 == 0)
  {
    stringstream a, b;
    b << std::setw(6) << std::setfill('0') << samples;
    a << saveName << b.str() << ".png";
    stbi_write_png(a.str().c_str(), windowWidth, windowHeight, 3, pixels, windowWidth * 3);
    printf("frame %d output \n", samples);
  }
}

RTCRayHit MyGUI::castRay(RTCRay ray, bool shadow)
{
  /*
   * The intersect context can be used to set intersection
   * filters or flags, and it also contains the instance ID stack
   * used in multi-level instancing.
   */
  struct RTCIntersectContext context;
  rtcInitIntersectContext(&context);

  /*
   * The ray hit structure holds both the ray and the hit.
   * The user must initialize it properly -- see API documentation
   * for rtcIntersect1() for details.
   */

  struct RTCRayHit rayhit;
  rayhit.ray = ray;
  rayhit.hit.geomID = RTC_INVALID_GEOMETRY_ID;
  rayhit.hit.instID[0] = RTC_INVALID_GEOMETRY_ID;

  /*
   * There are multiple variants of rtcIntersect. This one
   * intersects a single ray with the scene.
   */
  if (!shadow)
  {
    rtcIntersect1(sceneCam->scene, &context, &rayhit);
  }
  else if (shadow)
  {
    rtcOccluded1(sceneCam->scene, &context, &rayhit.ray);
  }

  return rayhit;
}

Eigen::Vector3f MyGUI::computeShading(Eigen::Vector3f incomingDir, Eigen::Vector3f intersection, Eigen::Vector3f normal, shared_ptr<nori::BSDF> material)
{

  // Return true if no shadow
  auto doShadowTest = [&](Eigen::Vector3f position, float range) -> bool {
    Eigen::Vector3f lightDir = (position - intersection);
    Eigen::Vector3f unitLightDir = lightDir.normalized();

    if (range == std::numeric_limits<float>::infinity())
    {
      range = lightDir.norm();
    }

    RTCRay lightRay;
    lightRay.org_x = intersection.x();
    lightRay.org_y = intersection.y();
    lightRay.org_z = intersection.z();

    lightRay.dir_x = unitLightDir.x();
    lightRay.dir_y = unitLightDir.y();
    lightRay.dir_z = unitLightDir.z();

    lightRay.tnear = .01;
    lightRay.tfar = range - .02;
    lightRay.flags = 0;
    lightRay.mask = 0;

    RTCRayHit rayHit = castRay(lightRay, true);

    return rayHit.ray.tfar != -std::numeric_limits<float>::infinity();
  };

  Eigen::Vector3f color(0, 0, 0);

  for (int i = 0; i < sceneCam->lights.size(); i++)
  {
    shared_ptr<BaseLight> light = sceneCam->lights[i];

    color += light->getContribution(incomingDir, intersection, normal, material, doShadowTest);
  }

  return color;
}

bool MyGUI::mouseMotionEvent(const Eigen::Vector2i &p, const Eigen::Vector2i &rel, int button, int modifiers)
{
  if (button == 1)
  {
    this->theta -= float(rel.x()) / 100.;
    this->phi -= float(rel.y()) / 100.;
    samples = 0;
    return true;
  }
  else
  {
    return false;
  }
}