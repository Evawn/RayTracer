#include <../RTUtil/ImgGUI.hpp>
#include <generator.h>
#include <../ext/embree/include/embree3/rtcore.h>

#include <sstream>
#include <iomanip>

class MyGUI : public RTUtil::ImgGUI
{

  shared_ptr<SceneAndCam> sceneCam;

public:
  MyGUI(string name, int width, int height, shared_ptr<SceneAndCam> s);
  ~MyGUI();

  string saveName;

  // int width, height;
  unsigned int samples = 0;

  // pixel array for output
  unsigned char *pixels;

  Eigen::Vector3f missColor;

  void computeImage();

  RTCRayHit castRay(RTCRay ray, bool shadow);

  Eigen::Vector3f computeShading(Eigen::Vector3f incomingDir, Eigen::Vector3f intersection, Eigen::Vector3f normal, std::shared_ptr<nori::BSDF> material);

  virtual bool mouseMotionEvent(const Eigen::Vector2i &p, const Eigen::Vector2i &rel, int button, int modifiers) override;

  // virtual bool scrollEvent(const Eigen::Vector2i &p, const Eigen::Vector2i &rel) override;

  // virtual bool keyboardEvent(int key, int scancode, int action, int modifiers) override;
  // bool keyboardEvent(int key, int scancode, int action, int modifiers);

private:
  float theta;
  float phi;
  float deltaZoom;
};